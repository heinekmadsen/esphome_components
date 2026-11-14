from __future__ import annotations

import json
import logging
from typing import Any, Callable

from aioesphomeapi import APIClient, TextSensorInfo, TextSensorState, UserService

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
        super().__init__(hass, _LOGGER, name=DOMAIN, update_interval=None)
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
        self.topology_key: int | None = None
        self.services: dict[str, UserService] = {}
        self.zones: dict[int, dict[str, Any]] = {}
        self._platforms: dict[str, tuple[Callable, EntityFactory]] = {}
        self._platform_entities: dict[str, dict[int, list[Entity]]] = {}

    async def async_setup(self) -> None:
        await self.client.connect(login=True)
        device_info = self.client.device_info
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

    async def async_shutdown(self) -> None:
        await self.client.disconnect()

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
                via_device=(DOMAIN, self.mac),
            )

    def zone_device_info(self, zone: dict[str, Any]) -> DeviceInfo:
        return DeviceInfo(
            identifiers={(DOMAIN, f"{self.mac}_zone_{zone['id']}")},
            name=zone.get("name") or f"Zone {zone['id']}",
            manufacturer="MoreGeekN",
            model="Wavin AHC9000 Zone",
            via_device=(DOMAIN, self.mac),
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
        await self.client.execute_service(service, data)
