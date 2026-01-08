
from esphome.components import climate
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from  .. import balboa_spa_ns, CONF_BALBOA_SPA_ID, BalboaSpa
#DEPENDENCIES = ["balboa_spa"]
BalboaClimate = balboa_spa_ns.class_("BalboaClimate", climate.Climate, cg.Component)

CONFIG_SCHEMA = cv.All(
    climate.climate_schema(BalboaClimate)
    .extend(
        {
            cv.GenerateID(CONF_BALBOA_SPA_ID): cv.use_id(BalboaSpa),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    parent = await cg.get_variable(config[CONF_BALBOA_SPA_ID])
    cg.add(var.set_balboa_parent(parent))