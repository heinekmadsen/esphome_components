import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor
from .. import WavinAhc9000, CONF_WAVINAHC9000_ID
from esphome.const import (
    CONF_ID,
    CONF_SENSOR,
)
 
wavinAhc9000_ns = cg.esphome_ns.namespace('wavinAhc9000')
WavinAhc9000Climate = wavinAhc9000_ns.class_('WavinAhc9000Climate', climate.Climate, cg.Component)
CONF_CHANNEL = "channel"

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(WavinAhc9000Climate),
    cv.GenerateID(CONF_WAVINAHC9000_ID): cv.use_id(WavinAhc9000),
    cv.Required(CONF_CHANNEL): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    wavinAhc9000 = yield cg.get_variable(config[CONF_WAVINAHC9000_ID])
    var = cg.new_Pvariable(config[CONF_ID], wavinAhc9000)
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)
 
    #chan = yield cg.get_variable(config[CONF_CHANNEL])
    #cg.add(var.set_channel(chan))
