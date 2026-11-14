from __future__ import annotations

from typing import Any

from homeassistant.components.sensor import (
    SensorDeviceClass,
    SensorEntity,
    SensorStateClass,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.const import PERCENTAGE, UnitOfTemperature
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
        "sensor",
        async_add_entities,
        lambda coordinator, zone: [
            WavinZoneTemperatureSensor(coordinator, zone),
            WavinZoneFloorSensor(coordinator, zone),
            WavinZoneBatterySensor(coordinator, zone),
        ],
    )


class WavinZoneTemperatureSensor(WavinZoneEntity, SensorEntity):
    _attr_name = "Temperature"
    _attr_device_class = SensorDeviceClass.TEMPERATURE
    _attr_native_unit_of_measurement = UnitOfTemperature.CELSIUS
    _attr_state_class = SensorStateClass.MEASUREMENT
    _attr_suggested_display_precision = 1
    _entity_key = "temperature"

    @property
    def native_value(self) -> float | None:
        value = self.zone.get("current")
        return float(value) if value is not None else None


class WavinZoneFloorSensor(WavinZoneEntity, SensorEntity):
    _attr_name = "Floor temperature"
    _attr_device_class = SensorDeviceClass.TEMPERATURE
    _attr_native_unit_of_measurement = UnitOfTemperature.CELSIUS
    _attr_state_class = SensorStateClass.MEASUREMENT
    _attr_suggested_display_precision = 1
    _entity_key = "floor_temperature"

    @property
    def native_value(self) -> float | None:
        value = self.zone.get("floor")
        return float(value) if value is not None else None


class WavinZoneBatterySensor(WavinZoneEntity, SensorEntity):
    _attr_name = "Battery"
    _attr_device_class = SensorDeviceClass.BATTERY
    _attr_native_unit_of_measurement = PERCENTAGE
    _attr_state_class = SensorStateClass.MEASUREMENT
    _attr_suggested_display_precision = 0
    _entity_key = "battery"

    @property
    def native_value(self) -> int | None:
        value = self.zone.get("battery")
        return int(value) if value is not None else None
