import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from .. import Nilan, CONF_NILAN_ID
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
    ICON_MOLECULE_CO2,
    UNIT_PARTS_PER_MILLION ,
    ICON_PERCENT,
    ICON_CHECK_CIRCLE_OUTLINE,
    ICON_GAUGE,
    CONF_BINARY_SENSOR,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_EMPTY,
)

DEPENDENCIES = ['nilan']

CONF_TEMP_T0 = "temp_t0"
CONF_TEMP_T1 = "temp_t1"
CONF_TEMP_T2 = "temp_t2"
CONF_TEMP_T3 = "temp_t3"
CONF_TEMP_T4 = "temp_t4"
CONF_TEMP_T5 = "temp_t5"
CONF_TEMP_T6 = "temp_t6"
CONF_TEMP_T7 = "temp_t7"
CONF_TEMP_T8 = "temp_t8"
CONF_TEMP_T9 = "temp_t9"
CONF_TEMP_T10 = "temp_t10"
CONF_TEMP_T11 = "temp_t11"
CONF_TEMP_T12 = "temp_t12"
CONF_TEMP_T13 = "temp_t13"
CONF_TEMP_T14 = "temp_t14"
CONF_TEMP_T15 = "temp_t15"
CONF_TEMP_T16 = "temp_t16"
CONF_TEMP_T17 = "temp_t17"
CONF_TEMP_T18 = "temp_t18"
CONF_MEASURED_HUMIDITY = "measured_humidity"
CONF_ACTIVE_ALARMS = "active_alarms"
CONF_COOL_TARGET_TEMP = "cool_target_temp"
CONF_TEMP_MIN_SUMMER = "min_summer_temp"
CONF_TEMP_MIN_WINTER = "min_winter_temp"
CONF_TEMP_MAX_SUMMER = "max_summer_temp"
CONF_TEMP_MAX_WINTER = "max_winter_temp"
CONF_IS_SUMMER = "is_summer"
CONF_HEAT_EXCHANGE_EFFICIENCY = "heat_exchange_efficiency"
CONF_CO2_LEVEL = "co2_ppm_level"
CONF_INLET_FAN = "inlet_fan"
CONF_EXHAUST_FAN = "exhaust_fan"
CONF_TARGET_TEMP = "target_temp"
CONF_HEAT = "heat"
CONF_TIMER = "timer"
CONF_VENTILATION_SPEED = "ventilation_speed"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_NILAN_ID): cv.use_id(Nilan),
    cv.Required(CONF_TEMP_T7): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Required(CONF_TARGET_TEMP): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Required(CONF_VENTILATION_SPEED): sensor.sensor_schema(UNIT_EMPTY, ICON_GAUGE, 1, DEVICE_CLASS_EMPTY),

    cv.Optional(CONF_TEMP_T0): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T1): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T2): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T3): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T4): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T5): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T6): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),    
    cv.Optional(CONF_TEMP_T8): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T9): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T10): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T11): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T12): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T13): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T14): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T15): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T16): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T17): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_T18): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_MEASURED_HUMIDITY): sensor.sensor_schema(UNIT_PERCENT, ICON_WATER_PERCENT, 1, DEVICE_CLASS_HUMIDITY),
    cv.Optional(CONF_ACTIVE_ALARMS): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 0, DEVICE_CLASS_EMPTY),
    cv.Optional(CONF_COOL_TARGET_TEMP): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_MIN_SUMMER): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_MAX_SUMMER): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_MIN_WINTER): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_TEMP_MAX_WINTER): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    cv.Optional(CONF_IS_SUMMER): sensor.sensor_schema(UNIT_EMPTY, ICON_CHECK_CIRCLE_OUTLINE, 0, DEVICE_CLASS_EMPTY),
    cv.Optional(CONF_HEAT_EXCHANGE_EFFICIENCY): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1, DEVICE_CLASS_EMPTY),
    cv.Optional(CONF_CO2_LEVEL): sensor.sensor_schema(UNIT_PARTS_PER_MILLION , ICON_MOLECULE_CO2 , 0, DEVICE_CLASS_EMPTY),
    cv.Optional(CONF_INLET_FAN): sensor.sensor_schema(UNIT_PERCENT , ICON_PERCENT , 0, DEVICE_CLASS_EMPTY),
    cv.Optional(CONF_EXHAUST_FAN): sensor.sensor_schema(UNIT_PERCENT , ICON_PERCENT , 0, DEVICE_CLASS_EMPTY),
    #cv.Optional(CONF_HEAT): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_EMPTY),
    #cv.Optional(CONF_TIMER): sensor.sensor_schema(UNIT_EMPTY, ICON_GAUGE, 1, DEVICE_CLASS_EMPTY)
}).extend(cv.polling_component_schema('60s'))


def to_code(config):
    nilan = yield cg.get_variable(config[CONF_NILAN_ID])

    if CONF_TEMP_T0 in config:
        conf = config[CONF_TEMP_T0]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t0_sensor(sens))
    if CONF_TEMP_T1 in config:
        conf = config[CONF_TEMP_T1]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t1_sensor(sens))
    if CONF_TEMP_T2 in config:
        conf = config[CONF_TEMP_T2]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t2_sensor(sens))
    if CONF_TEMP_T3 in config:
        conf = config[CONF_TEMP_T3]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t3_sensor(sens))
    if CONF_TEMP_T4 in config:
        conf = config[CONF_TEMP_T4]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t4_sensor(sens))
    if CONF_TEMP_T5 in config:
        conf = config[CONF_TEMP_T5]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t5_sensor(sens))
    if CONF_TEMP_T6 in config:
        conf = config[CONF_TEMP_T6]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t6_sensor(sens))
    if CONF_TEMP_T7 in config:
        conf = config[CONF_TEMP_T7]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t7_sensor(sens))
    if CONF_TEMP_T8 in config:
        conf = config[CONF_TEMP_T8]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t8_sensor(sens))
    if CONF_TEMP_T9 in config:
        conf = config[CONF_TEMP_T9]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t9_sensor(sens))
    if CONF_TEMP_T10 in config:
        conf = config[CONF_TEMP_T10]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t10_sensor(sens))
    if CONF_TEMP_T11 in config:
        conf = config[CONF_TEMP_T11]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t11_sensor(sens))
    if CONF_TEMP_T12 in config:
        conf = config[CONF_TEMP_T12]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t12_sensor(sens))
    if CONF_TEMP_T13 in config:
        conf = config[CONF_TEMP_T13]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t13_sensor(sens))
    if CONF_TEMP_T14 in config:
        conf = config[CONF_TEMP_T14]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t14_sensor(sens))
    if CONF_TEMP_T15 in config:
        conf = config[CONF_TEMP_T15]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t15_sensor(sens))
    if CONF_TEMP_T16 in config:
        conf = config[CONF_TEMP_T16]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t16_sensor(sens))
    if CONF_TEMP_T17 in config:
        conf = config[CONF_TEMP_T17]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t17_sensor(sens))
    if CONF_TEMP_T18 in config:
        conf = config[CONF_TEMP_T18]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_temp_t18_sensor(sens))
    if CONF_MEASURED_HUMIDITY in config:
        conf = config[CONF_MEASURED_HUMIDITY]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_measured_humidity_sensor(sens))
    if CONF_ACTIVE_ALARMS in config:
        conf = config[CONF_ACTIVE_ALARMS]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_active_alarms_sensor(sens))
    if CONF_COOL_TARGET_TEMP in config:
        conf = config[CONF_COOL_TARGET_TEMP]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_cool_target_temp_sensor(sens))
    if CONF_TEMP_MIN_SUMMER in config:
        conf = config[CONF_TEMP_MIN_SUMMER]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_min_summer_temp_sensor(sens))
    if CONF_TEMP_MAX_SUMMER in config:
        conf = config[CONF_TEMP_MAX_SUMMER]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_max_summer_temp_sensor(sens))
    if CONF_TEMP_MIN_WINTER in config:
        conf = config[CONF_TEMP_MIN_WINTER]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_min_winter_temp_sensor(sens))
    if CONF_TEMP_MAX_WINTER in config:
        conf = config[CONF_TEMP_MAX_WINTER]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_max_winter_temp_sensor(sens))
    if CONF_IS_SUMMER in config:
        conf = config[CONF_IS_SUMMER]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_is_summer_sensor(sens))
    if CONF_HEAT_EXCHANGE_EFFICIENCY in config:
        conf = config[CONF_HEAT_EXCHANGE_EFFICIENCY]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_heat_exchange_efficiency_sensor(sens))
    if CONF_CO2_LEVEL in config:
        conf = config[CONF_CO2_LEVEL]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_co2_sensor(sens))
    if CONF_INLET_FAN in config:
        conf = config[CONF_INLET_FAN]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_inlet_fan_sensor(sens))
    if CONF_EXHAUST_FAN in config:
        conf = config[CONF_EXHAUST_FAN]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_exhaust_fan_sensor(sens))
    if CONF_TARGET_TEMP in config:
        conf = config[CONF_TARGET_TEMP]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_target_temp_sensor(sens))
    if CONF_VENTILATION_SPEED in config:
        conf = config[CONF_VENTILATION_SPEED]
        sens = yield sensor.new_sensor(conf)
        cg.add(nilan.set_ventilation_speed_sensor(sens))
        
