import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor
from .. import WavinAhc9000, CONF_GENVEX_ID
from esphome.const import (
    CONF_ID,
    CONF_SENSOR,
)
 
wavinAhc9000_ns = cg.esphome_ns.namespace('wavinAhc9000')
WavinAhc9000Climate = wavinAhc9000_ns.class_('WavinAhc9000Climate', climate.Climate, cg.Component)
 
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(WavinAhc9000Climate),
    cv.GenerateID(CONF_GENVEX_ID): cv.use_id(WavinAhc9000),
    cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    wavinAhc9000 = yield cg.get_variable(config[CONF_GENVEX_ID])
    var = cg.new_Pvariable(config[CONF_ID], wavinAhc9000)
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)
 
    sens = yield cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_sensor(sens))
