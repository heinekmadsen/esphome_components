import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor, text_sensor
from .. import Nilan, CONF_NILAN_ID
from esphome.const import (
    CONF_ID
)

CONF_TARGET_TEMP = "target_temp_sensor_id"
CONF_CURRENT_TEMP = "current_temp_sensor_id"
CONF_FAN_SPEED = "fan_speed_sensor_id"
CONF_MODE = "mode_text_sensor_id"

nilan_ns = cg.esphome_ns.namespace('nilan')
NilanClimate = nilan_ns.class_('NilanClimate', climate.Climate, cg.Component)
 
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(NilanClimate),
    cv.GenerateID(CONF_NILAN_ID): cv.use_id(Nilan),
    cv.Required(CONF_TARGET_TEMP): cv.use_id(sensor.Sensor),
    cv.Required(CONF_CURRENT_TEMP): cv.use_id(sensor.Sensor),
    cv.Required(CONF_FAN_SPEED): cv.use_id(sensor.Sensor),
    cv.Required(CONF_MODE): cv.use_id(text_sensor.TextSensor)
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    nilan = yield cg.get_variable(config[CONF_NILAN_ID])
    var = cg.new_Pvariable(config[CONF_ID], nilan)
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)

    sens_set_temp = yield cg.get_variable(config[CONF_TARGET_TEMP])
    cg.add(var.set_temp_setpoint_sensor(sens_set_temp))

    sens_current_temp = yield cg.get_variable(config[CONF_CURRENT_TEMP])
    cg.add(var.set_current_temp_sensor(sens_current_temp))

    sens_fan_speed = yield cg.get_variable(config[CONF_FAN_SPEED])
    cg.add(var.set_fan_speed_sensor(sens_fan_speed))

    sens_mode = yield cg.get_variable(config[CONF_MODE])
    cg.add(var.set_mode_sensor(sens_mode))