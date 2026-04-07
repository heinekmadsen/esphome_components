import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_DEVICE_CLASS,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ICON,
    CONF_ACCURACY_DECIMALS,
    UNIT_PERCENT,
    DEVICE_CLASS_BATTERY,
    ICON_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    UNIT_CELSIUS,
)

from .. import WavinAHC9000

CONF_PARENT_ID = "wavinahc9000v3_id"
CONF_CHANNEL = "channel"


CONF_TYPE = "type"

CONFIG_SCHEMA = sensor.sensor_schema().extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Required(CONF_TYPE): cv.one_of(
            "battery",
            "temperature",
            "comfort_setpoint",
            "floor_temperature",
            "floor_min_temperature",
            "floor_max_temperature",
            lower=True,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])

    # Inject sensor metadata defaults based on type before creating the sensor.
    # ESPHome 2026.x removed set_device_class/set_unit_of_measurement/set_icon
    # from the C++ Sensor class; these must now be set via the config dict.
    stype = config[CONF_TYPE]
    if stype == "battery":
        config.setdefault(CONF_DEVICE_CLASS, DEVICE_CLASS_BATTERY)
        config.setdefault(CONF_UNIT_OF_MEASUREMENT, UNIT_PERCENT)
        config.setdefault(CONF_ICON, ICON_BATTERY)
        config.setdefault(CONF_ACCURACY_DECIMALS, 0)
    else:
        config.setdefault(CONF_DEVICE_CLASS, DEVICE_CLASS_TEMPERATURE)
        config.setdefault(CONF_UNIT_OF_MEASUREMENT, UNIT_CELSIUS)
        config.setdefault(CONF_ACCURACY_DECIMALS, 1)

    sens = await sensor.new_sensor(config)

    if stype == "battery":
        cg.add(hub.add_channel_battery_sensor(config[CONF_CHANNEL], sens))
    elif stype == "comfort_setpoint":
        cg.add(hub.add_channel_comfort_setpoint_sensor(config[CONF_CHANNEL], sens))
    elif stype == "floor_temperature":
        cg.add(hub.add_channel_floor_temperature_sensor(config[CONF_CHANNEL], sens))
    elif stype == "floor_min_temperature":
        cg.add(hub.add_channel_floor_min_temperature_sensor(config[CONF_CHANNEL], sens))
    elif stype == "floor_max_temperature":
        cg.add(hub.add_channel_floor_max_temperature_sensor(config[CONF_CHANNEL], sens))
    else:
        cg.add(hub.add_channel_temperature_sensor(config[CONF_CHANNEL], sens))
    cg.add(hub.add_active_channel(config[CONF_CHANNEL]))
