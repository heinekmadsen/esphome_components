import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins, automation
from esphome.automation import maybe_simple_id
from esphome.const import CONF_ID, CONF_ADDRESS, CONF_UPDATE_INTERVAL

wavinAhc9000_ns = cg.esphome_ns.namespace('wavinAhc9000')
WavinAhc9000 = wavinAhc9000_ns.class_('WavinAhc9000', cg.PollingComponent)

CONF_WAVINAHC9000_ID = 'wavinAhc9000_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WavinAhc9000),
    #cv.Required(CONF_ADDRESS): cv.int_range(min=1, max=100),
    
}).extend(cv.polling_component_schema('60s'))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    
    #if CONF_ADDRESS in config:
    #    cg.add(var.set_address(config[CONF_ADDRESS]))
    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))    