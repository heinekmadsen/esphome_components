import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins, automation
from esphome.automation import maybe_simple_id
from esphome.const import CONF_ID, CONF_ADDRESS, CONF_UPDATE_INTERVAL

genvex_ns = cg.esphome_ns.namespace('genvex')
Genvex = genvex_ns.class_('Genvex', cg.Component)

CONF_GENVEX_ID = 'genvex_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_GENVEX_ID): cv.use_id(Genvex),
    cv.Required(CONF_ADDRESS): cv.string,
    cv.Required(CONF_UPDATE_INTERVAL): cv.int_range(min=1, max=100),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    
    if CONF_ADDRESS in config:
        cg.add(var.set_address(config[CONF_ADDRESS]))
    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))    