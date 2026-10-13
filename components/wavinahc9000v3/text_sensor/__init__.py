import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from .. import WavinAHC9000

CONF_PARENT_ID = "wavinahc9000v3_id"
CONF_TYPE = "type"

# Publishes which channels are in use and which zone owns each one. Updated by
# the `wavin_discover_channels` service; used by the commissioning flow.
CONFIG_SCHEMA = text_sensor.text_sensor_schema(
    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Optional(CONF_TYPE, default="discovery"): cv.one_of("discovery", lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = await text_sensor.new_text_sensor(config)
    cg.add(hub.set_discovery_text_sensor(var))
