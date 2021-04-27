import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from .. import WavinAhc9000, CONF_WAVINAHC9000_ID
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = ['wavinAhc9000']

CONF_CHANNEL_C1 = "c1"
CONF_CHANNEL_C2 = "c2"
CONF_CHANNEL_C3 = "c3"
CONF_CHANNEL_C4 = "c4"
CONF_CHANNEL_C5 = "c5"
CONF_CHANNEL_C6 = "c6"
CONF_CHANNEL_C7 = "c7"
CONF_CHANNEL_C8 = "c8"
CONF_CHANNEL_C9 = "c9"
CONF_CHANNEL_C10 = "c10"
CONF_CHANNEL_C11 = "c11"
CONF_CHANNEL_C12 = "c12"
CONF_CHANNEL_C13 = "c13"
CONF_CHANNEL_C14 = "c14"
CONF_CHANNEL_C15 = "c15"
CONF_CHANNEL_C16 = "c16"

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(CONF_WAVINAHC9000_ID): cv.use_id(WavinAhc9000),
    cv.Optional(CONF_CHANNEL_C1): cv.int_,
    cv.Optional(CONF_CHANNEL_C2): cv.int_,
    cv.Optional(CONF_CHANNEL_C3): cv.int_,
    cv.Optional(CONF_CHANNEL_C4): cv.int_,
    cv.Optional(CONF_CHANNEL_C5): cv.int_,
    cv.Optional(CONF_CHANNEL_C6): cv.int_,
    cv.Optional(CONF_CHANNEL_C7): cv.int_,
    cv.Optional(CONF_CHANNEL_C8): cv.int_,
    cv.Optional(CONF_CHANNEL_C9): cv.int_,
    cv.Optional(CONF_CHANNEL_C10): cv.int_,
    cv.Optional(CONF_CHANNEL_C11): cv.int_,
    cv.Optional(CONF_CHANNEL_C12): cv.int_,
    cv.Optional(CONF_CHANNEL_C13): cv.int_,
    cv.Optional(CONF_CHANNEL_C14): cv.int_,
    cv.Optional(CONF_CHANNEL_C15): cv.int_,
    cv.Optional(CONF_CHANNEL_C16): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    wavinAhc9000 = yield cg.get_variable(config[CONF_WAVINAHC9000_ID])
    var = cg.new_Pvariable(config[CONF_ID], wavinAhc9000)
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)