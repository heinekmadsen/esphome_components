import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_CHANNEL

from .. import WavinAHC9000, ns

CONF_PARENT_ID = "wavinahc9000v3_id"
CONF_TYPE = "type"

WavinChildLockSwitch = ns.class_("WavinChildLockSwitch", switch.Switch)

# Only one switch type currently; keep 'type' for forward compatibility.
CONFIG_SCHEMA = switch.switch_schema(WavinChildLockSwitch).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Optional(CONF_TYPE, default="child_lock"): cv.one_of("child_lock", lower=True),
    }
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    ch = config[CONF_CHANNEL]
    var = await switch.new_switch(config)
    cg.add(var.set_parent(hub))
    cg.add(var.set_channel(ch))
    cg.add(hub.add_channel_child_lock_switch(ch, var))
    # Optimistic publish handled in write_state override; hub refresh will reconcile.
