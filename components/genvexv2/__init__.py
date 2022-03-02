import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

genvexv2_ns = cg.esphome_ns.namespace('genvexv2')
Genvexv2 = genvexv2_ns.class_('Genvexv2', cg.Component)

CONF_GENVEXV2_ID = 'genvexv2_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Genvexv2),   
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config) 