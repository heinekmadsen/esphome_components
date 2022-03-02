import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor, select, number
from .. import Genvexv2, CONF_GENVEXV2_ID
from esphome.const import (
    CONF_ID
)

CONF_TARGET_TEMP = "target_temp_sensor_id"
CONF_CURRENT_TEMP = "current_temp_sensor_id"
CONF_FAN_SPEED = "fan_speed_sensor_id"
#CONF_MODE = "mode_select_id"

genvexv2_ns = cg.esphome_ns.namespace('genvexv2')
Genvexv2Climate = genvexv2_ns.class_('Genvexv2Climate', climate.Climate, cg.Component)
 
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(Genvexv2Climate),
    cv.GenerateID(CONF_GENVEXV2_ID): cv.use_id(Genvexv2),
    cv.Required(CONF_TARGET_TEMP): cv.use_id(number.Number),
    cv.Required(CONF_CURRENT_TEMP): cv.use_id(sensor.Sensor),
    cv.Required(CONF_FAN_SPEED): cv.use_id(number.Number),
    #cv.Required(CONF_MODE): cv.use_id(select.Select)
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)

    number_set_temp = yield cg.get_variable(config[CONF_TARGET_TEMP])
    cg.add(var.set_temp_setpoint_number(number_set_temp))

    sens_current_temp = yield cg.get_variable(config[CONF_CURRENT_TEMP])
    cg.add(var.set_current_temp_sensor(sens_current_temp))

    number_fan_speed = yield cg.get_variable(config[CONF_FAN_SPEED])
    cg.add(var.set_fan_speed_number(number_fan_speed))

    #select_mode = yield cg.get_variable(config[CONF_MODE])
    #cg.add(var.set_mode_select(select_mode))