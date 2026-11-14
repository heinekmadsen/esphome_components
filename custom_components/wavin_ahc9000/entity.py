from __future__ import annotations

from typing import Any

from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
from .coordinator import WavinCoordinator


class WavinZoneEntity(CoordinatorEntity[WavinCoordinator]):
    """Base class for an entity belonging to one Wavin zone."""

    _attr_has_entity_name = True
    _entity_key = "zone"

    def __init__(self, coordinator: WavinCoordinator, zone: dict[str, Any]) -> None:
        super().__init__(coordinator)
        self._zone_id = int(zone["id"])
        self._attr_unique_id = f"{coordinator.mac}_zone_{self._zone_id}_{self._entity_key}"

    @property
    def zone(self) -> dict[str, Any]:
        return self.coordinator.zones.get(self._zone_id, {})

    @property
    def available(self) -> bool:
        return super().available and bool(self.zone)

    @property
    def device_info(self) -> DeviceInfo:
        return self.coordinator.zone_device_info(self.zone)

    @property
    def extra_state_attributes(self) -> dict[str, Any]:
        return {
            "channels": self.zone.get("channels"),
            "zone_id": self._zone_id,
        }
