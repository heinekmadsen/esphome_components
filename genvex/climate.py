import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import climate, sensor, modbus
from esphome.const import (
    CONF_ID,
    CONF_SENSOR, 
    CONF_TARGET_TEMPERATURE,
    CONF_FAN_MODE_ON_ACTION,
    CONF_FAN_MODE_OFF_ACTION,
    CONF_FAN_MODE_AUTO_ACTION,
    CONF_FAN_MODE_LOW_ACTION,
    CONF_FAN_MODE_MEDIUM_ACTION,
    CONF_FAN_MODE_HIGH_ACTION,
    CONF_FAN_MODE_MIDDLE_ACTION,
)

AUTO_LOAD = ['modbus']

genvex_ns = cg.esphome_ns.namespace('genvex')
GenvexClimate = genvex_ns.class_('GenvexClimate', climate.Climate, cg.Component, modbus.ModbusDevice)



CONFIG_SCHEMA = cv.All(climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(GenvexClimate),
    cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
    cv.Required(CONF_TARGET_TEMPERATURE): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_FAN_MODE_ON_ACTION): automation.validate_automation(
        single=True
    ),
    cv.Optional(CONF_FAN_MODE_OFF_ACTION): automation.validate_automation(
        single=True
    ),
    cv.Optional(CONF_FAN_MODE_AUTO_ACTION): automation.validate_automation(
        single=True
    ),
    cv.Optional(CONF_FAN_MODE_LOW_ACTION): automation.validate_automation(
        single=True
    ),
    cv.Optional(CONF_FAN_MODE_MEDIUM_ACTION): automation.validate_automation(
        single=True
    ),
    cv.Optional(CONF_FAN_MODE_HIGH_ACTION): automation.validate_automation(
        single=True
    ),
    cv.Optional(CONF_FAN_MODE_MIDDLE_ACTION): automation.validate_automation(
        single=True
    )

}).extend(cv.COMPONENT_SCHEMA))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)

    sens = yield cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_sensor(sens))

    target_temp = yield cg.get_variable(config[CONF_TARGET_TEMPERATURE])
    cg.add(var.set_sensor(target_temp))