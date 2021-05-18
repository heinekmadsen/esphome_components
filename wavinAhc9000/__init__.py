import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins
from esphome.components import modbus
from esphome.const import CONF_ID, CONF_RW_PIN

wavinAhc9000_ns = cg.esphome_ns.namespace('wavinAhc9000')
WavinAhc9000 = wavinAhc9000_ns.class_('WavinAhc9000', cg.PollingComponent)

CONF_WAVINAHC9000_ID = 'wavinAhc9000_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WavinAhc9000),
    cv.Required(CONF_RW_PIN): pins.gpio_output_pin_schema
}).extend(cv.polling_component_schema('60s')).extend(modbus.modbus_device_schema(0x01))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield modbus.register_modbus_device(var, config)
    pin = yield cg.gpio_pin_expression(config[CONF_RW_PIN])
    cg.add(var.set_rw_pin(pin))
