from __future__ import annotations

from typing import Any

from homeassistant.components.climate import (
    ClimateEntity,
    ClimateEntityFeature,
    HVACAction,
    HVACMode,
)
from homeassistant.components.climate.const import ATTR_TEMPERATURE
from homeassistant.config_entries import ConfigEntry
from homeassistant.const import UnitOfTemperature
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
        "climate",
        async_add_entities,
        lambda coordinator, zone: [WavinClimateEntity(coordinator, zone)],
    )


class WavinClimateEntity(WavinZoneEntity, ClimateEntity):
    _attr_name = None
    _attr_temperature_unit = UnitOfTemperature.CELSIUS
    _attr_target_temperature_step = 0.5
    _attr_min_temp = 5.0
    _attr_max_temp = 35.0
    _attr_hvac_modes = [HVACMode.HEAT, HVACMode.OFF]
    _attr_supported_features = ClimateEntityFeature.TARGET_TEMPERATURE
    _entity_key = "climate"

    @property
    def current_temperature(self) -> float | None:
        value = self.zone.get("current")
        return float(value) if value is not None else None

    @property
    def target_temperature(self) -> float | None:
        value = self.zone.get("target")
        return float(value) if value is not None else None

    @property
    def hvac_mode(self) -> HVACMode:
        return HVACMode.OFF if self.zone.get("off") else HVACMode.HEAT

    @property
    def hvac_action(self) -> HVACAction:
        if self.zone.get("off"):
            return HVACAction.OFF
        return (
            HVACAction.HEATING
            if self.zone.get("action") == "heating"
            else HVACAction.IDLE
        )

    async def async_set_temperature(self, **kwargs: Any) -> None:
        temperature = kwargs.get(ATTR_TEMPERATURE)
        if temperature is None:
            return
        await self.coordinator.call_service(
            "wavin_zone_set_target",
            {"zone": self._zone_id, "temperature": float(temperature)},
        )

    async def async_set_hvac_mode(self, hvac_mode: HVACMode) -> None:
        await self.coordinator.call_service(
            "wavin_zone_set_mode",
            {"zone": self._zone_id, "turn_off": hvac_mode == HVACMode.OFF},
        )
