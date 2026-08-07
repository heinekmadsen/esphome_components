import logging

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import modbus
from esphome.const import CONF_ID, CONF_RW_PIN

_LOGGER = logging.getLogger(__name__)

wavinAhc9000_ns = cg.esphome_ns.namespace('wavinAhc9000')
WavinAhc9000 = wavinAhc9000_ns.class_('WavinAhc9000', cg.PollingComponent, modbus.ModbusClientDevice)

CONF_WAVINAHC9000_ID = 'wavinAhc9000_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WavinAhc9000),
    # Deprecated: RS-485 direction control is now handled by the modbus hub via
    # `flow_control_pin`, because sends are queued/async and this component can no
    # longer toggle the DE pin itself. Accepted but ignored for config compatibility.
    cv.Optional(CONF_RW_PIN): pins.gpio_output_pin_schema,
}).extend(cv.polling_component_schema('60s')).extend(modbus.modbus_device_schema(0x01))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_client_device(var, config)
    if CONF_RW_PIN in config:
        _LOGGER.warning(
            "wavinAhc9000: 'rw_pin' is deprecated and ignored. Move RS-485 direction "
            "control to the modbus hub: 'modbus: { flow_control_pin: <pin> }'."
        )
