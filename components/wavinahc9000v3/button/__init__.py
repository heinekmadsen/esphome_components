import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from .. import WavinAHC9000, ns

CONF_PARENT_ID = "wavin_ahc9000_id"
CONF_CHANNEL = "channel"

WavinRepairButton = ns.class_("WavinRepairButton", button.Button, cg.Component)
WavinYamlDumpButton = ns.class_("WavinYamlDumpButton", button.Button)

# Optional extended repair clears additional flags that may lock keypad
CONF_EXTENDED = "extended"
CONF_AGGRESSIVE = "aggressive"
CONF_NORMALIZE = "normalize"
CONF_NORMALIZE_OFF = "normalize_off"

_REPAIR_SCHEMA = button.button_schema(WavinRepairButton).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Optional(CONF_EXTENDED, default=False): cv.boolean,
        cv.Optional(CONF_AGGRESSIVE, default=False): cv.boolean,
        cv.Optional(CONF_NORMALIZE, default=False): cv.boolean,
        cv.Optional(CONF_NORMALIZE_OFF, default=False): cv.boolean,
    }
)

_DUMP_SCHEMA = button.button_schema(WavinYamlDumpButton).extend(
    {cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000)}
)

CONFIG_SCHEMA = cv.Any(_REPAIR_SCHEMA, _DUMP_SCHEMA)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    btn = await button.new_button(config)
    cg.add(btn.set_parent(hub))

    if CONF_CHANNEL in config:
        cg.add(btn.set_channel(config[CONF_CHANNEL]))
        cg.add(btn.set_extended(config[CONF_EXTENDED]))
        cg.add(btn.set_aggressive(config[CONF_AGGRESSIVE]))
        cg.add(btn.set_normalize(config[CONF_NORMALIZE]))
        cg.add(btn.set_normalize_off(config[CONF_NORMALIZE_OFF]))
        cg.add(hub.add_active_channel(config[CONF_CHANNEL]))
