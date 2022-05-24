import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

sentio_ns = cg.esphome_ns.namespace('sentio')
Sentio = sentio_ns.class_('Sentio', cg.Component)

CONF_SENTIO_ID = 'sentio_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Sentio),   
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config) 