from __future__ import annotations

from typing import Any

from homeassistant.components.switch import SwitchEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers import entity_registry as er
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import DOMAIN
from .coordinator import WavinCoordinator
from .entity import WavinParentEntity, WavinZoneEntity


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    coordinator: WavinCoordinator = hass.data[DOMAIN][entry.entry_id]
    async_add_entities([WavinSyncSwitch(coordinator)])
    coordinator.register_platform(
        "switch",
        async_add_entities,
        lambda coordinator, zone: [WavinChildLockSwitch(coordinator, zone)],
    )


class WavinChildLockSwitch(WavinZoneEntity, SwitchEntity):
    _attr_name = "Child lock"
    _entity_key = "child_lock"

    @property
    def is_on(self) -> bool:
        return bool(self.zone.get("child_lock"))

    async def async_turn_on(self, **kwargs: Any) -> None:
        await self.coordinator.call_service(
            "wavin_zone_set_child_lock",
            {"zone": self._zone_id, "enabled": True},
        )

    async def async_turn_off(self, **kwargs: Any) -> None:
        await self.coordinator.call_service(
            "wavin_zone_set_child_lock",
            {"zone": self._zone_id, "enabled": False},
        )

class WavinSyncSwitch(WavinParentEntity, SwitchEntity):
    _attr_has_entity_name = False
    _entity_key = "sync"

    async def async_added_to_hass(self) -> None:
        await super().async_added_to_hass()
        if self.entity_id:
            er.async_get(self.hass).async_update_entity(
                self.entity_id, has_entity_name=False, name=self.coordinator.sync_label
            )

    @property
    def name(self) -> str:
        return self.coordinator.sync_label

    @property
    def is_on(self) -> bool:
        return self.coordinator.sync_enabled

    async def async_turn_on(self, **kwargs: Any) -> None:
        await self.coordinator.call_service("wavin_zone_set_sync", {"enabled": True})

    async def async_turn_off(self, **kwargs: Any) -> None:
        await self.coordinator.call_service("wavin_zone_set_sync", {"enabled": False})

    @callback
    def _handle_coordinator_update(self) -> None:
        super()._handle_coordinator_update()
        if self.entity_id:
            er.async_get(self.hass).async_update_entity(
                self.entity_id, name=self.coordinator.sync_label
            )
