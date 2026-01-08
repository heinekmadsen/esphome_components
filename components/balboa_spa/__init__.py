import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, binary_sensor, text_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_TEMPERATURE, UNIT_CELSIUS, STATE_CLASS_MEASUREMENT
DEPENDENCIES = ['uart']
AUTO_LOAD = ['uart', 'sensor', 'text_sensor', 'binary_sensor']

balboa_spa_ns = cg.esphome_ns.namespace('balboa_spa')

BalboaSpa = balboa_spa_ns.class_('BalboaSpa', cg.Component, uart.UARTDevice)

CONF_BALBOA_SPA_ID = "balboa_spa_id"

CONF_TEMP_SENSOR = "temp_sensor"
CONF_HEATER_SENSOR = "heater_binary_sensor"
CONF_CIRC_SENSOR = "circ_binary_sensor"
CONF_BLOWER_SENSOR ="blower_binary_sensor"
CONF_REST_SENSOR ="rest_binary_sensor"
CONF_FAULT_TEXT_SENSOR = "fault_text_sensor"
CONF_HOUR_SENSOR = "hour_sensor"
CONF_MINUTE_SENSOR = "minute_sensor"
CONF_TIME_TEXT_SENSOR = "time_text_sensor"
CONF_HEATING_STATUS_TEXT_SENSOR = "heating_status_text_sensor"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BalboaSpa), 
    cv.Optional(CONF_TEMP_SENSOR):
        sensor.sensor_schema(device_class=DEVICE_CLASS_TEMPERATURE,unit_of_measurement=UNIT_CELSIUS,accuracy_decimals=1,state_class=STATE_CLASS_MEASUREMENT).extend(), 
    cv.Optional(CONF_CIRC_SENSOR):
        binary_sensor.binary_sensor_schema().extend(), 
    cv.Optional(CONF_HEATER_SENSOR):
        binary_sensor.binary_sensor_schema().extend(), 
    cv.Optional(CONF_BLOWER_SENSOR):
        binary_sensor.binary_sensor_schema().extend(), 
    cv.Optional(CONF_REST_SENSOR):
        binary_sensor.binary_sensor_schema().extend(), 
    cv.Optional(CONF_FAULT_TEXT_SENSOR):
        text_sensor.text_sensor_schema().extend(), 
    cv.Optional(CONF_HOUR_SENSOR):
        sensor.sensor_schema(accuracy_decimals=0, unit_of_measurement="h").extend(),
    cv.Optional(CONF_MINUTE_SENSOR):
        sensor.sensor_schema(accuracy_decimals=0, unit_of_measurement="min").extend(),
    cv.Optional(CONF_TIME_TEXT_SENSOR):
        text_sensor.text_sensor_schema().extend(),
    cv.Optional(CONF_HEATING_STATUS_TEXT_SENSOR):
        text_sensor.text_sensor_schema().extend(),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add_library(name="rlogiacco/CircularBuffer", version=None)
    await uart.register_uart_device(var, config)



    if CONF_TEMP_SENSOR in config:
        conf = config[CONF_TEMP_SENSOR]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temp_sensor(sens))
    if CONF_CIRC_SENSOR in config:
        conf = config[CONF_CIRC_SENSOR]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_circ_binary_sensor(sens))
    if CONF_HEATER_SENSOR in config:
        conf = config[CONF_HEATER_SENSOR]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_heater_binary_sensor(sens))
    if CONF_BLOWER_SENSOR in config:
        conf = config[CONF_BLOWER_SENSOR]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_blower_binary_sensor(sens))
    if CONF_REST_SENSOR in config:
        conf = config[CONF_REST_SENSOR]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_rest_binary_sensor(sens))
    if CONF_FAULT_TEXT_SENSOR in config:
        conf = config[CONF_FAULT_TEXT_SENSOR]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_fault_text_sensor(sens))
    if CONF_HOUR_SENSOR in config:
        conf = config[CONF_HOUR_SENSOR]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_hour_sensor(sens))
    if CONF_MINUTE_SENSOR in config:
        conf = config[CONF_MINUTE_SENSOR]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_minute_sensor(sens))
    if CONF_TIME_TEXT_SENSOR in config:
        conf = config[CONF_TIME_TEXT_SENSOR]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_time_text_sensor(sens))
    if CONF_HEATING_STATUS_TEXT_SENSOR in config:
        conf = config[CONF_HEATING_STATUS_TEXT_SENSOR]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_heating_status_text_sensor(sens))
