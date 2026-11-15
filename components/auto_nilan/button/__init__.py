import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from .. import AutoNilan, ns

CONF_PARENT_ID = "auto_nilan_id"
CONF_ACTION = "action"

AutoNilanButton = ns.class_("AutoNilanButton", button.Button, cg.Component)

ACTION_COMMISSION = 0
ACTION_RESET_ALARM = 1
ACTION_RESET_FILTER = 2

ACTIONS = {
    "commission": ACTION_COMMISSION,
    "reset_alarm": ACTION_RESET_ALARM,
    "reset_filter": ACTION_RESET_FILTER,
}

CONFIG_SCHEMA = button.button_schema(AutoNilanButton).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(AutoNilan),
        cv.Optional(CONF_ACTION, default="commission"): cv.enum(ACTIONS, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = await button.new_button(config)
    cg.add(var.set_parent(hub))
    cg.add(var.set_action(config[CONF_ACTION]))
