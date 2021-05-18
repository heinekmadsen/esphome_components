import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor
from .. import wavinAhc9000_ns, WavinAhc9000, CONF_WAVINAHC9000_ID
from esphome.const import (
    CONF_ID,
    CONF_CHANNEL,
)
 
WavinAhc9000Climate = wavinAhc9000_ns.class_('WavinAhc9000Climate', climate.Climate, cg.Component)
 
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(WavinAhc9000Climate),
    cv.GenerateID(CONF_WAVINAHC9000_ID): cv.use_id(WavinAhc9000),
    cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=16),
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    wavin = yield cg.get_variable(config[CONF_WAVINAHC9000_ID])
    var = cg.new_Pvariable(config[CONF_ID], wavin)
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)
 
    cg.add(var.set_channel(config[CONF_CHANNEL]))
