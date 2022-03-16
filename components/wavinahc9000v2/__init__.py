import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

wavinahc9000v2_ns = cg.esphome_ns.namespace('wavinahc9000v2')
Wavinahc9000v2 = wavinahc9000v2_ns.class_('Wavinahc9000v2', cg.Component)

CONF_WAVINAHC9000v2_ID = 'wavinahc9000v2_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Wavinahc9000v2),   
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config) 
