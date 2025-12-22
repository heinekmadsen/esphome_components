import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_NAME

from .. import WavinAHC9000, WavinSetpointNumber

CONF_PARENT_ID = "wavinahc9000v3_id"
CONF_CHANNEL = "channel"
CONF_TYPE = "type"

SETPOINT_TYPES = ["comfort", "standby"]

CONFIG_SCHEMA = number.number_schema(WavinSetpointNumber).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Required(CONF_TYPE): cv.one_of(*SETPOINT_TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await number.register_number(var, config, min_value=5, max_value=35, step=0.5)
    cg.add(var.set_parent(hub))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    if config[CONF_TYPE] == "comfort":
        cg.add(var.set_type(WavinSetpointNumber.Type.COMFORT))
        cg.add(hub.add_comfort_number(var))
    else:
        cg.add(var.set_type(WavinSetpointNumber.Type.STANDBY))
        cg.add(hub.add_standby_number(var))
    cg.add(hub.add_active_channel(config[CONF_CHANNEL]))
