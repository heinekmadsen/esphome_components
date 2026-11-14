from __future__ import annotations

import asyncio
from datetime import timedelta
import json
import logging
from typing import Any, Callable

from aioesphomeapi import (
    APIClient,
    ConnectionClosedEvent,
    TextSensorInfo,
    TextSensorState,
    UserService,
)
from aioesphomeapi.core import ConnectionNotEstablishedAPIError

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers import device_registry as dr
from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator
from homeassistant.helpers.entity import Entity

from .const import CONF_HOST, CONF_NOISE_PSK, CONF_PORT, DOMAIN, TOPOLOGY_NAME

_LOGGER = logging.getLogger(__name__)

EntityFactory = Callable[["WavinCoordinator", dict[str, Any]], list[Entity]]


class WavinCoordinator(DataUpdateCoordinator[dict[int, dict[str, Any]]]):
    """Own the ESPHome native API connection and the zone topology."""

    def __init__(self, hass: HomeAssistant, entry: ConfigEntry) -> None:
        super().__init__(hass, _LOGGER, name=DOMAIN, update_interval=timedelta(seconds=30))
        self.entry = entry
        self.client = APIClient(
            entry.data[CONF_HOST],
            entry.data[CONF_PORT],
            None,
            noise_psk=entry.data.get(CONF_NOISE_PSK) or None,
        )
        self.mac: str = ""
        self.device_name: str = ""
        self.sw_version: str | None = None
        self.parent_device_id: str | None = None
        self.topology_key: int | None = None
        self.services: dict[str, UserService] = {}
        self.zones: dict[int, dict[str, Any]] = {}
        self.rssi: int | None = None
        self.sync_enabled = False
        self.master_zone: int | None = None
        self.master_name = ""
        self._platforms: dict[str, tuple[Callable, EntityFactory]] = {}
        self._platform_entities: dict[str, dict[int, list[Entity]]] = {}
        self._reconnect_task: asyncio.Task | None = None
        self._shutdown = False
        self._remove_connection_closed = self.client.add_connection_closed_callback(
            self._on_connection_closed
        )

    async def async_setup(self) -> None:
        await self._async_connect_and_discover()

    async def _async_connect_and_discover(self) -> None:
        await self.client.connect(on_stop=self._on_stop, login=True)
        device_info = await self.client.device_info()
        if device_info is None:
            raise RuntimeError("ESPHome device information is unavailable")
        self.mac = device_info.mac_address
        self.device_name = device_info.name
        self.sw_version = device_info.project_version or device_info.esphome_version

        entities, services = await self.client.list_entities_services()
        self.services = {service.name: service for service in services}

        for info in entities:
            if isinstance(info, TextSensorInfo) and (
                info.name == TOPOLOGY_NAME
                or "topology" in (getattr(info, "object_id", "") or "").lower()
            ):
                self.topology_key = info.key
                break
        if self.topology_key is None:
            raise RuntimeError("No Wavin topology text sensor found")

        self.client.subscribe_states(self._on_state)

        device_registry = dr.async_get(self.hass)
        device_registry.async_get_or_create(
            config_entry_id=self.entry.entry_id,
            identifiers={(DOMAIN, self.mac)},
            name=self.device_name,
            manufacturer="MoreGeekN",
            model="Wavin AHC9000",
            sw_version=self.sw_version,
        )

    async def _async_update_data(self) -> dict[int, dict[str, Any]]:
        if not self.client.is_connected:
            try:
                await self._async_connect_and_discover()
            except Exception as err:  # noqa: BLE001
                _LOGGER.warning("Reconnect failed: %s", err)
        return self.zones

    async def async_shutdown(self) -> None:
        self._shutdown = True
        self._remove_connection_closed()
        if self._reconnect_task is not None:
            self._reconnect_task.cancel()
            self._reconnect_task = None
        await self.client.disconnect()

    @callback
    def _on_connection_closed(self, event: ConnectionClosedEvent) -> None:
        if self._shutdown:
            return
        _LOGGER.warning("ESPHome connection closed (%s); scheduling reconnect", event)
        if self._reconnect_task is None or self._reconnect_task.done():
            self._reconnect_task = self.hass.async_create_task(self._async_reconnect())

    async def _on_stop(self, expected: bool) -> None:
        if self._shutdown:
            return
        _LOGGER.warning("ESPHome connection stopped; reconnecting")
        self._reconnect_task = self.hass.async_create_task(self._async_reconnect())

    async def _async_reconnect(self) -> None:
        while not self._shutdown:
            try:
                await asyncio.sleep(3)
                await self._async_connect_and_discover()
                _LOGGER.info("Reconnected to Wavin ESPHome device")
                return
            except asyncio.CancelledError:
                raise
            except Exception as err:  # noqa: BLE001
                _LOGGER.warning("Reconnect failed: %s", err)
                await asyncio.sleep(5)

    @callback
    def _on_state(self, state: TextSensorState) -> None:
        if state.key != self.topology_key:
            return
        if getattr(state, "missing_state", False):
            return
        payload = getattr(state, "state", None)
        if not payload:
            return
        try:
            parsed = json.loads(payload)
        except (TypeError, ValueError):
            _LOGGER.warning("Invalid Wavin topology JSON: %s", payload)
            return
        zones = {
            int(zone["id"]): zone
            for zone in parsed.get("zones", [])
            if isinstance(zone, dict) and "id" in zone
        }
        self.rssi = parsed.get("rssi")
        self.sync_enabled = bool(parsed.get("sync"))
        self.master_zone = parsed.get("master_zone") or None
        self.master_name = parsed.get("master_name") or ""
        self.zones = zones
        self._ensure_zone_devices()
        self.async_set_updated_data(zones)
        self._sync_all_platforms()

    def _ensure_zone_devices(self) -> None:
        device_registry = dr.async_get(self.hass)
        for zone in self.zones.values():
            device_registry.async_get_or_create(
                config_entry_id=self.entry.entry_id,
                identifiers={(DOMAIN, f"{self.mac}_zone_{zone['id']}")},
                name=zone.get("name") or f"Zone {zone['id']}",
                manufacturer="MoreGeekN",
                model="Wavin AHC9000 Zone",
                via_device_id=self.parent_device_id,
            )

    @property
    def sync_label(self) -> str:
        name = self.master_name.strip()
        if name.lower().startswith("zone "):
            name = "zone " + name[5:]
        return f"Sync all zones to {name}" if name else "Sync all zones"

    def parent_device_info(self) -> DeviceInfo:
        return DeviceInfo(
            identifiers={(DOMAIN, self.mac)},
            name=self.device_name,
            manufacturer="MoreGeekN",
            model="Wavin AHC9000",
            sw_version=self.sw_version,
        )

    def zone_device_info(self, zone: dict[str, Any]) -> DeviceInfo:
        return DeviceInfo(
            identifiers={(DOMAIN, f"{self.mac}_zone_{zone['id']}")},
            name=zone.get("name") or f"Zone {zone['id']}",
            manufacturer="MoreGeekN",
            model="Wavin AHC9000 Zone",
            via_device_id=self.parent_device_id,
        )

    def register_platform(
        self, key: str, add_entities: Callable, factory: EntityFactory
    ) -> None:
        self._platforms[key] = (add_entities, factory)
        self._sync_platform(key)

    def _sync_all_platforms(self) -> None:
        for key in self._platforms:
            self._sync_platform(key)

    @callback
    def _sync_platform(self, key: str) -> None:
        add_entities, factory = self._platforms[key]
        existing = self._platform_entities.setdefault(key, {})
        wanted: dict[int, list[Entity]] = {}

        for zone_id, zone in self.zones.items():
            entities = factory(self, zone)
            if entities:
                wanted[zone_id] = entities

        for zone_id, entities in wanted.items():
            if zone_id not in existing:
                add_entities(entities)
                existing[zone_id] = entities

        for zone_id in list(existing):
            if zone_id not in wanted:
                for entity in existing.pop(zone_id):
                    self.hass.async_create_task(entity.async_remove())

    async def call_service(self, name: str, data: dict[str, Any]) -> None:
        service = self.services.get(name)
        if service is None:
            _LOGGER.warning("ESPHome service %s is not available", name)
            return
        try:
            await self.client.execute_service(service, data)
        except ConnectionNotEstablishedAPIError:
            _LOGGER.warning("ESPHome connection lost during service call; reconnecting")
            await self._async_connect_and_discover()
            service = self.services.get(name)
            if service is not None:
                await self.client.execute_service(service, data)
