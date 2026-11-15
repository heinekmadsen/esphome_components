import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@nic6911"]
AUTO_LOAD = ["uart", "text_sensor"]

ns = cg.esphome_ns.namespace("sentiov2")
SentioV2 = ns.class_("SentioV2", cg.PollingComponent, uart.UARTDevice)

CONF_UART_ID = "uart_id"
CONF_FLOW_CONTROL_PIN = "flow_control_pin"
CONF_ADDRESS = "address"
CONF_RECEIVE_TIMEOUT_MS = "receive_timeout_ms"
CONF_INTER_FRAME_DELAY_US = "inter_frame_delay_us"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SentioV2),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_ADDRESS, default=1): cv.int_range(min=1, max=247),
            cv.Optional(CONF_RECEIVE_TIMEOUT_MS, default=300): cv.positive_int,
            cv.Optional(CONF_INTER_FRAME_DELAY_US, default=4000): cv.int_range(
                min=0, max=50000
            ),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("10s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await uart.register_uart_device(var, config)
    await cg.register_component(var, config)

    if CONF_FLOW_CONTROL_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_receive_timeout_ms(config[CONF_RECEIVE_TIMEOUT_MS]))
    cg.add(var.set_inter_frame_delay_us(config[CONF_INTER_FRAME_DELAY_US]))
