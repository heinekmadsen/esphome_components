import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, modbus
from esphome.const import CONF_ID, CONF_TEMPERATURE, ICON_THERMOMETER, \
    UNIT_CELSIUS, CONF_HUMIDITY, ICON_WATER_PERCENT, UNIT_PERCENT, ICON_EMPTY, UNIT_EMPTY, ICON_PERCENT, ICON_CHECK_CIRCLE_OUTLINE, ICON_GAUGE, \
    CONF_BINARY_SENSOR

AUTO_LOAD = ['modbus']

genvex_ns = cg.esphome_ns.namespace('genvex')
Genvex = genvex_ns.class_('Genvex', cg.PollingComponent, modbus.ModbusDevice)

CONF_TEMP_T1 = "temp_t1"
CONF_TEMP_T2 = "temp_t2"
CONF_TEMP_T3 = "temp_t3"
CONF_TEMP_T4 = "temp_t4"
CONF_TEMP_T5 = "temp_t5"
CONF_TEMP_T6 = "temp_t6"
CONF_TEMP_T7 = "temp_t7"
CONF_TEMP_T8 = "temp_t8"
CONF_TEMP_T9 = "temp_t9"
CONF_TEMP_T2_PANEL = "temp_t2_panel"
CONF_MEASURED_HUMIDITY = "measured_humidity"
CONF_HUMIDITY_CALCULATED_SETPOINT = "humidity_calculated_setpoint"
CONF_ALARM_BIT = "alarm_bit"
CONF_INLET_FAN = "inlet_fan"
CONF_EXTRACT_FAN = "extract_fan"
CONF_BYPASS = "bypass"
CONF_WATERVALVE = "watervalve"
CONF_HUMIDITY_FAN_CONTROL = "humidity_fan_control"
CONF_BYPASS_ON_OFF = "bypass_on_off"
CONF_TARGET_TEMP = "target_temp"
CONF_SPEED_MODE = "speed_mode"
CONF_HEAT = "heat"
CONF_TIMER = "timer"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Genvex),
    cv.Optional(CONF_TEMP_T1): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T2): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T3): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T4): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T5): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T6): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T7): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T8): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T9): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TEMP_T2_PANEL): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_MEASURED_HUMIDITY): sensor.sensor_schema(UNIT_PERCENT, ICON_WATER_PERCENT, 1),
    cv.Optional(CONF_HUMIDITY_CALCULATED_SETPOINT): sensor.sensor_schema(UNIT_PERCENT, ICON_WATER_PERCENT, 1),
    cv.Optional(CONF_ALARM_BIT): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1),
    cv.Optional(CONF_INLET_FAN): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1),
    cv.Optional(CONF_EXTRACT_FAN): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1),
    cv.Optional(CONF_BYPASS): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1),
    cv.Optional(CONF_WATERVALVE): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1),
    cv.Optional(CONF_HUMIDITY_FAN_CONTROL): sensor.sensor_schema(UNIT_PERCENT, ICON_PERCENT, 1),
    cv.Optional(CONF_BYPASS_ON_OFF): sensor.sensor_schema(UNIT_EMPTY, ICON_CHECK_CIRCLE_OUTLINE, 1),
    cv.Optional(CONF_TARGET_TEMP): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_SPEED_MODE): sensor.sensor_schema(UNIT_EMPTY, ICON_GAUGE, 1),
    cv.Optional(CONF_HEAT): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_TIMER): sensor.sensor_schema(UNIT_EMPTY, ICON_GAUGE, 1)

}).extend(cv.polling_component_schema('60s')).extend(modbus.modbus_device_schema(0x01))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield modbus.register_modbus_device(var, config)

    if CONF_TEMP_T1 in config:
        conf = config[CONF_TEMP_T1]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t1_sensor(sens))
    if CONF_TEMP_T2 in config:
        conf = config[CONF_TEMP_T2]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t2_sensor(sens))
    if CONF_TEMP_T3 in config:
        conf = config[CONF_TEMP_T3]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t3_sensor(sens))
    if CONF_TEMP_T4 in config:
        conf = config[CONF_TEMP_T4]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t4_sensor(sens))
    if CONF_TEMP_T5 in config:
        conf = config[CONF_TEMP_T5]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t5_sensor(sens))
    if CONF_TEMP_T6 in config:
        conf = config[CONF_TEMP_T6]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t6_sensor(sens))
    if CONF_TEMP_T7 in config:
        conf = config[CONF_TEMP_T7]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t7_sensor(sens))
    if CONF_TEMP_T8 in config:
        conf = config[CONF_TEMP_T8]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t8_sensor(sens))
    if CONF_TEMP_T9 in config:
        conf = config[CONF_TEMP_T9]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t9_sensor(sens))
    if CONF_TEMP_T2_PANEL in config:
        conf = config[CONF_TEMP_T2_PANEL]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temp_t2_panel_sensor(sens))
    if CONF_MEASURED_HUMIDITY in config:
        conf = config[CONF_MEASURED_HUMIDITY]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_measured_humidity_sensor(sens))
    if CONF_HUMIDITY_CALCULATED_SETPOINT in config:
        conf = config[CONF_HUMIDITY_CALCULATED_SETPOINT]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_humidity_calculated_setpoint_sensor(sens))
    if CONF_ALARM_BIT in config:
        conf = config[CONF_ALARM_BIT]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_alarm_bit_sensor(sens))
    if CONF_INLET_FAN in config:
        conf = config[CONF_INLET_FAN]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_inlet_fan_sensor(sens))
    if CONF_EXTRACT_FAN in config:
        conf = config[CONF_EXTRACT_FAN]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_extract_fan_sensor(sens))
    if CONF_BYPASS in config:
        conf = config[CONF_BYPASS]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_bypass_sensor(sens))
    if CONF_WATERVALVE in config:
        conf = config[CONF_WATERVALVE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_watervalve_sensor(sens))
    if CONF_HUMIDITY_FAN_CONTROL in config:
        conf = config[CONF_HUMIDITY_FAN_CONTROL]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_humidity_fan_control_sensor(sens))
    if CONF_BYPASS_ON_OFF in config:
        conf = config[CONF_BYPASS_ON_OFF]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_bypass_on_off_sensor(sens))
    if CONF_TARGET_TEMP in config:
        conf = config[CONF_TARGET_TEMP]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_target_temp_sensor(sens))
    if CONF_SPEED_MODE in config:
        conf = config[CONF_SPEED_MODE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_speed_mode_sensor(sens))
    if CONF_HEAT in config:
        conf = config[CONF_HEAT]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_heat_sensor(sens))
    if CONF_TIMER in config:
        conf = config[CONF_TIMER]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_timer_sensor(sens))          
        
