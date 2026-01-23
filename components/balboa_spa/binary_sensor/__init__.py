
from esphome.components import binary_sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from .. import balboa_spa_ns, CONF_BALBOA_SPA_ID, BalboaSpa

BalboaBinarySensor = balboa_spa_ns.class_(
    "BalboaBinarySensor", binary_sensor.BinarySensor, cg.Component
)

CONF_BINARY_SENSOR_DATAPOINT = "binary_sensor_datapoint"

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(BalboaBinarySensor)
    .extend(
        {
            cv.GenerateID(CONF_BALBOA_SPA_ID): cv.use_id(BalboaSpa),
            cv.Required(CONF_BINARY_SENSOR_DATAPOINT): cv.positive_int,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)
async def to_code(config):
    parent = await cg.get_variable(config[CONF_BALBOA_SPA_ID])

    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)

    cg.add(var.set_balboa_parent(parent))

    cg.add(var.set_binary_sensor_id(config[CONF_BINARY_SENSOR_DATAPOINT]))