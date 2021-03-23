import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor
from .. import Genvex, CONF_GENVEX_ID
from esphome.const import (
    CONF_ID,
    CONF_SENSOR,
)
 
genvex_ns = cg.esphome_ns.namespace('genvex')
GenvexClimate = genvex_ns.class_('GenvexClimate', climate.Climate, cg.Component)
 
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(GenvexClimate),
    cv.GenerateID(CONF_GENVEX_ID): cv.use_id(Genvex),
    cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    genvex = yield cg.get_variable(config[CONF_GENVEX_ID])
    var = cg.new_Pvariable(config[CONF_ID], genvex)
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)
 
    sens = yield cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_sensor(sens))
