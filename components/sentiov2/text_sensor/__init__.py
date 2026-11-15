import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from .. import SentioV2

CONF_PARENT_ID = "sentiov2_id"
CONF_TYPE = "type"

CONFIG_SCHEMA = text_sensor.text_sensor_schema(
    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(SentioV2),
        cv.Optional(CONF_TYPE, default="topology"): cv.one_of(
            "topology", lower=True
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = await text_sensor.new_text_sensor(config)
    cg.add(hub.set_topology_text_sensor(var))
