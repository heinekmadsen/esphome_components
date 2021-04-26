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

CONF_TEMP_C1 = "temp_c1"
CONF_TEMP_C2 = "temp_c2"
CONF_TEMP_C3 = "temp_c3"
CONF_TEMP_C4 = "temp_c4"
CONF_TEMP_C5 = "temp_c5"
CONF_TEMP_C6 = "temp_c6"
CONF_TEMP_C7 = "temp_c7"
CONF_TEMP_C8 = "temp_c8"
CONF_TEMP_C9 = "temp_c9"
CONF_TEMP_C10 = "temp_c10"
CONF_TEMP_C11 = "temp_c11"
CONF_TEMP_C12 = "temp_c12"
CONF_TEMP_C13 = "temp_c13"
CONF_TEMP_C14 = "temp_c14"
CONF_TEMP_C15 = "temp_c15"
CONF_TEMP_C16 = "temp_c16"

CONF_BATTERY_C1 = "bat_c1"
CONF_BATTERY_C2 = "bat_c2"
CONF_BATTERY_C3 = "bat_c3"
CONF_BATTERY_C4 = "bat_c4"
CONF_BATTERY_C5 = "bat_c5"
CONF_BATTERY_C6 = "bat_c6"
CONF_BATTERY_C7 = "bat_c7"
CONF_BATTERY_C8 = "bat_c8"
CONF_BATTERY_C9 = "bat_c9"
CONF_BATTERY_C10 = "bat_c10"
CONF_BATTERY_C11 = "bat_c11"
CONF_BATTERY_C12 = "bat_c12"
CONF_BATTERY_C13 = "bat_c13"
CONF_BATTERY_C14 = "bat_c14"
CONF_BATTERY_C15 = "bat_c15"
CONF_BATTERY_C16 = "bat_c16"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WAVINAHC9000_ID): cv.use_id(WavinAhc9000),
    cv.Optional(CONF_TEMP_C1): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C1): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C2): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C2): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C3): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C3): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C4): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C4): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C5): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C5): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C6): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C6): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C7): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C7): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C8): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C8): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C9): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C9): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C10): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C10): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C11): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C11): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C12): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C12): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C13): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C13): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C14): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C14): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C15): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C15): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
    cv.Optional(CONF_TEMP_C16): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_BATTERY_C16): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_BATTERY),
}).extend(cv.polling_component_schema('60s'))


def to_code(config):
    wavinAhc9000 = yield cg.get_variable(config[CONF_WAVINAHC9000_ID])

    if CONF_TEMP_C1 in config:
        conf = config[CONF_TEMP_C1]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c1_sensor(sens))
    if CONF_BATTERY_C1 in config:
        conf = config[CONF_BATTERY_C1]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c1_sensor(sens))
    if CONF_TEMP_C2 in config:
        conf = config[CONF_TEMP_C2]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c2_sensor(sens))
    if CONF_BATTERY_C2 in config:
        conf = config[CONF_BATTERY_C2]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c2_sensor(sens))
    if CONF_TEMP_C3 in config:
        conf = config[CONF_TEMP_C3]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c3_sensor(sens))
    if CONF_BATTERY_C3 in config:
        conf = config[CONF_BATTERY_C3]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c3_sensor(sens))
    if CONF_TEMP_C4 in config:
        conf = config[CONF_TEMP_C4]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c4_sensor(sens))
    if CONF_BATTERY_C4 in config:
        conf = config[CONF_BATTERY_C4]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c4_sensor(sens))
    if CONF_TEMP_C5 in config:
        conf = config[CONF_TEMP_C5]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c5_sensor(sens))
    if CONF_BATTERY_C5 in config:
        conf = config[CONF_BATTERY_C5]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c5_sensor(sens))
    if CONF_TEMP_C6 in config:
        conf = config[CONF_TEMP_C6]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c6_sensor(sens))
    if CONF_BATTERY_C6 in config:
        conf = config[CONF_BATTERY_C6]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c6_sensor(sens))
    if CONF_TEMP_C7 in config:
        conf = config[CONF_TEMP_C7]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c7_sensor(sens))
    if CONF_BATTERY_C7 in config:
        conf = config[CONF_BATTERY_C7]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c7_sensor(sens))        
    if CONF_TEMP_C8 in config:
        conf = config[CONF_TEMP_C8]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c8_sensor(sens))
    if CONF_BATTERY_C8 in config:
        conf = config[CONF_BATTERY_C8]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c8_sensor(sens))
    if CONF_TEMP_C9 in config:
        conf = config[CONF_TEMP_C9]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c9_sensor(sens))
    if CONF_BATTERY_C9 in config:
        conf = config[CONF_BATTERY_C9]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c9_sensor(sens))
    if CONF_TEMP_C10 in config:
        conf = config[CONF_TEMP_C10]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c10_sensor(sens))
    if CONF_BATTERY_C10 in config:
        conf = config[CONF_BATTERY_C10]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c10_sensor(sens))
    if CONF_TEMP_C11 in config:
        conf = config[CONF_TEMP_C11]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c11_sensor(sens))
    if CONF_BATTERY_C11 in config:
        conf = config[CONF_BATTERY_C11]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c11_sensor(sens))
    if CONF_TEMP_C12 in config:
        conf = config[CONF_TEMP_C12]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c12_sensor(sens))
    if CONF_BATTERY_C12 in config:
        conf = config[CONF_BATTERY_C12]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c12_sensor(sens))
    if CONF_TEMP_C13 in config:
        conf = config[CONF_TEMP_C13]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c13_sensor(sens))
    if CONF_BATTERY_C13 in config:
        conf = config[CONF_BATTERY_C13]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c13_sensor(sens))
    if CONF_TEMP_C14 in config:
        conf = config[CONF_TEMP_C14]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c14_sensor(sens))
    if CONF_BATTERY_C14 in config:
        conf = config[CONF_BATTERY_C14]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c14_sensor(sens))
    if CONF_TEMP_C15 in config:
        conf = config[CONF_TEMP_C15]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c15_sensor(sens))
    if CONF_BATTERY_C15 in config:
        conf = config[CONF_BATTERY_C15]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c15_sensor(sens))
    if CONF_TEMP_C16 in config:
        conf = config[CONF_TEMP_C16]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_temp_c16_sensor(sens))
    if CONF_BATTERY_C16 in config:
        conf = config[CONF_BATTERY_C16]
        sens = yield sensor.new_sensor(conf)
        cg.add(wavinAhc9000.set_bat_c16_sensor(sens))