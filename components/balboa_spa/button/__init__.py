
from esphome.components import button
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from .. import balboa_spa_ns, CONF_BALBOA_SPA_ID, BalboaSpa

BalboaButton = balboa_spa_ns.class_("BalboaButton", button.Button, cg.Component)

CONF_BUTTON_DATAPOINT = "button_datapoint"

CONFIG_SCHEMA = cv.All(
    button.button_schema(BalboaButton)
    .extend(
        {
            cv.GenerateID(CONF_BALBOA_SPA_ID): cv.use_id(BalboaSpa),
            cv.Required(CONF_BUTTON_DATAPOINT): cv.positive_int,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)
async def to_code(config):
    parent = await cg.get_variable(config[CONF_BALBOA_SPA_ID])

    var = await button.new_button(config)
    await cg.register_component(var, config)

    cg.add(var.set_balboa_parent(parent))

    cg.add(var.set_button_id(config[CONF_BUTTON_DATAPOINT]))