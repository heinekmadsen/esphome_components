import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from .. import WavinAHC9000

CONF_PARENT_ID = "wavinahc9000v4_id"
CONF_TYPE = "type"

# discovery: human/automation-oriented channel ownership report.
# topology: machine-readable JSON consumed by the custom Home Assistant integration.
CONFIG_SCHEMA = text_sensor.text_sensor_schema(
    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Optional(CONF_TYPE, default="discovery"): cv.one_of("discovery", "topology", lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = await text_sensor.new_text_sensor(config)
    if config[CONF_TYPE] == "topology":
        cg.add(hub.set_topology_text_sensor(var))
    else:
        cg.add(hub.set_discovery_text_sensor(var))
