import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID, CONF_NAME

from .. import WavinAHC9000, WavinZoneClimate

CONF_PARENT_ID = "wavinahc9000v3_id"
CONF_CHANNEL = "channel"
CONF_MEMBERS = "members"


CONF_STRICT_MODE_WRITES = "strict_mode_writes"

CONF_USE_FLOOR_TEMPERATURE = "use_floor_temperature"

CONFIG_SCHEMA = climate.climate_schema(WavinZoneClimate).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Optional(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Optional(CONF_MEMBERS): cv.ensure_list(cv.int_range(min=1, max=16)),
        cv.Optional(CONF_STRICT_MODE_WRITES, default=False): cv.boolean,
        cv.Optional(CONF_USE_FLOOR_TEMPERATURE, default=False): cv.boolean,
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await climate.register_climate(var, config)
    # Bind to hub
    cg.add(var.set_parent(hub))
    if CONF_CHANNEL in config:
        cg.add(var.set_single_channel(config[CONF_CHANNEL]))
        cg.add(hub.add_active_channel(config[CONF_CHANNEL]))
        if config[CONF_STRICT_MODE_WRITES]:
            cg.add(hub.set_strict_mode_write(config[CONF_CHANNEL], True))
        if config[CONF_USE_FLOOR_TEMPERATURE]:
            cg.add(var.set_use_floor_temperature(True))
        cg.add(hub.add_channel_climate(var))
    if CONF_MEMBERS in config:
        cg.add(var.set_members(config[CONF_MEMBERS]))
        for ch in config[CONF_MEMBERS]:
            cg.add(hub.add_active_channel(ch))
        cg.add(hub.add_group_climate(var))
