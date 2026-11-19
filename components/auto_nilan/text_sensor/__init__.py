import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from .. import AutoNilan

CONF_PARENT_ID = "auto_nilan_id"
CONF_TYPE = "type"

TYPES = ["topology", "values", "status"]

CONFIG_SCHEMA = text_sensor.text_sensor_schema(
    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(AutoNilan),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])
    var = await text_sensor.new_text_sensor(config)
    sensor_type = config[CONF_TYPE]
    if sensor_type == "topology":
        cg.add(hub.set_topology_text_sensor(var))
    elif sensor_type == "values":
        cg.add(hub.set_values_text_sensor(var))
    else:
        cg.add(hub.set_status_text_sensor(var))
