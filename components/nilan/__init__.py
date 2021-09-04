import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins, automation
from esphome.automation import maybe_simple_id
from esphome.const import CONF_ID, CONF_ADDRESS, CONF_UPDATE_INTERVAL
from esphome.components import modbus

nilan_ns = cg.esphome_ns.namespace('nilan')
Nilan = nilan_ns.class_('Nilan', cg.PollingComponent, modbus.ModbusDevice)

CONF_NILAN_ID = 'nilan_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Nilan),
    cv.Required(CONF_ADDRESS): cv.int_range(min=1, max=100),
    
}).extend(cv.polling_component_schema('60s')).extend(modbus.modbus_device_schema(0x01))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield modbus.register_modbus_device(var, config)
    
    if CONF_ADDRESS in config:
        cg.add(var.set_address(config[CONF_ADDRESS]))
    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))    