import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@nic6911"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor", "button"]

ns = cg.esphome_ns.namespace("auto_nilan")
AutoNilan = ns.class_("AutoNilan", cg.PollingComponent, uart.UARTDevice)

CONF_ADDRESS = "address"
CONF_FLOW_CONTROL_PIN = "flow_control_pin"
CONF_RECEIVE_TIMEOUT = "receive_timeout"
CONF_INTER_FRAME_DELAY = "inter_frame_delay"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AutoNilan),
            cv.Optional(CONF_ADDRESS, default=30): cv.int_range(min=1, max=247),
            # Only needed when the UART bus does not already drive DE/RE itself.
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RECEIVE_TIMEOUT, default="600ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_INTER_FRAME_DELAY, default=4000): cv.int_range(min=0, max=50000),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("30s"))
)


def _ms(value):
    total_ms = getattr(value, "total_milliseconds", None)
    if callable(total_ms):
        return int(total_ms())
    if total_ms is not None:
        return int(total_ms)
    return int(value.total_seconds() * 1000)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await uart.register_uart_device(var, config)
    await cg.register_component(var, config)

    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_receive_timeout_ms(_ms(config[CONF_RECEIVE_TIMEOUT])))
    cg.add(var.set_inter_frame_delay_us(config[CONF_INTER_FRAME_DELAY]))

    if CONF_FLOW_CONTROL_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))
