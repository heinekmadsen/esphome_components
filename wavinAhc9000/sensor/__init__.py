import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from .. import WavinAhc9000, CONF_WAVINAHC9000_ID
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    ICON_THERMOMETER,
    UNIT_CELSIUS,
    CONF_HUMIDITY,
    ICON_WATER_PERCENT,
    UNIT_PERCENT,
    ICON_EMPTY,
    UNIT_EMPTY,
    ICON_PERCENT,
    ICON_CHECK_CIRCLE_OUTLINE,
    ICON_GAUGE,
    CONF_BINARY_SENSOR,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_BATTERY,
)

DEPENDENCIES = ['wavinAhc9000']

CONF_TEMP_T8 = "temp_t8"
CONF_BATTERY_B8 = "bat_b8"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WAVINAHC9000_ID): cv.use_id(WavinAhc9000),
    cv.Required(CONF_TEMP_T8): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Required(CONF_BATTERY_B8): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
}).extend(cv.polling_component_schema('60s'))


def to_code(config):
    wavinAhc9000 = yield cg.get_variable(config[CONF_WAVINAHC9000_ID])

    if CONF_TEMP_T8 in config:
        conf = config[CONF_TEMP_T8]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_t8_sensor(sens))
    if CONF_BATTERY_B8 in config:
        conf = config[CONF_BATTERY_B8]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_b8_sensor(sens))
