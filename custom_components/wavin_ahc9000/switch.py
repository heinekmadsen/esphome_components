from __future__ import annotations

from typing import Any

from homeassistant.components.switch import SwitchEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import DOMAIN
from .coordinator import WavinCoordinator
from .entity import WavinZoneEntity


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    coordinator: WavinCoordinator = hass.data[DOMAIN][entry.entry_id]
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
