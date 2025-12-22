import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, climate, number
from esphome.const import CONF_ID
from esphome import pins

CODEOWNERS = ["@you"]
# Ensure dependent component code is compiled so their headers are available.
AUTO_LOAD = ["climate", "uart", "sensor", "number", "switch", "button"]

ns = cg.esphome_ns.namespace("wavin_ahc9000")
WavinAHC9000 = ns.class_("WavinAHC9000", cg.PollingComponent, uart.UARTDevice)
WavinZoneClimate = ns.class_("WavinZoneClimate", climate.Climate, cg.Component)
WavinSetpointNumber = ns.class_("WavinSetpointNumber", number.Number)
ModuleProfile = ns.enum("ModuleProfile")

CONF_UART_ID = "uart_id"
CONF_TX_ENABLE_PIN = "tx_enable_pin"
CONF_FLOW_CONTROL_PIN = "flow_control_pin"
CONF_TEMP_DIVISOR = "temp_divisor"
CONF_RECEIVE_TIMEOUT_MS = "receive_timeout_ms"
CONF_POLL_CHANNELS_PER_CYCLE = "poll_channels_per_cycle"
CONF_ALLOW_MODE_WRITES = "allow_mode_writes"
CONF_MODULE = "module"

MODULE_OPTIONS = {
    "default": ModuleProfile.MODULE_DEFAULT,
    "ustepper": ModuleProfile.MODULE_USTEPPER,
}

_FRIENDLY_NAME_KEYS = {
    cv.Optional(f"channel_{i:02d}_friendly_name"): cv.string for i in range(1, 17)
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(WavinAHC9000),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_TX_ENABLE_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_TEMP_DIVISOR, default=10.0): cv.positive_float,
            cv.Optional(CONF_RECEIVE_TIMEOUT_MS, default=1000): cv.positive_int,
            cv.Optional(CONF_POLL_CHANNELS_PER_CYCLE, default=2): cv.int_range(min=1, max=16),
            cv.Optional(CONF_ALLOW_MODE_WRITES, default=True): cv.boolean,
            cv.Optional(CONF_MODULE, default="default"): cv.enum(MODULE_OPTIONS, upper=False),
            **_FRIENDLY_NAME_KEYS,
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("5s"))
)



async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await uart.register_uart_device(var, config)
    await cg.register_component(var, config)
    if CONF_TX_ENABLE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_TX_ENABLE_PIN])
        cg.add(var.set_tx_enable_pin(pin))
    if CONF_FLOW_CONTROL_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))
    if CONF_TEMP_DIVISOR in config:
        cg.add(var.set_temp_divisor(config[CONF_TEMP_DIVISOR]))
    if CONF_RECEIVE_TIMEOUT_MS in config:
        cg.add(var.set_receive_timeout_ms(config[CONF_RECEIVE_TIMEOUT_MS]))
    if CONF_POLL_CHANNELS_PER_CYCLE in config:
        cg.add(var.set_poll_channels_per_cycle(config[CONF_POLL_CHANNELS_PER_CYCLE]))
    if CONF_ALLOW_MODE_WRITES in config:
        cg.add(var.set_allow_mode_writes(config[CONF_ALLOW_MODE_WRITES]))
    if CONF_MODULE in config:
        cg.add(var.set_module_profile(config[CONF_MODULE]))

    # Parse channel friendly names
    for key, value in config.items():
        if not isinstance(key, str):
            continue
        if key.startswith("channel_") and key.endswith("_friendly_name"):
            mid = key[len("channel_") : -len("_friendly_name")]
            # Accept both zero-padded and non-padded integers 1..16
            try:
                ch = int(mid)
            except ValueError:
                continue
            if 1 <= ch <= 16:
                cg.add(var.set_channel_friendly_name(ch, value))
