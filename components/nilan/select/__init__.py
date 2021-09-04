import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import select
from .. import Nilan, CONF_NILAN_ID
from esphome.const import (
    CONF_ID,
    CONF_INITIAL_OPTION,
    CONF_LAMBDA,
    CONF_OPTIONS,
    CONF_OPTIMISTIC,
    CONF_RESTORE_VALUE,
)

DEPENDENCIES = ['nilan']

#Select entities to HA
CONF_USER_FUNC_MODE = "user_func_mode"
CONF_USER_TIME_MODE = "user_time_mode"
CONF_USER_COOL_MODE = "user_cool_mode"
CONF_WEEK_PROGRAM = "week_program"

CONF_SET_ACTION = "set_action"
CONF_USER_FUNC_MODE_OPTIONS = ['none', 'extend', 'inlet', 'exhaust', 'external_heater_offset', 'ventilate']
CONF_USER_TIME_MODE_OPTIONS = ['000', '030', '100', '130', '200', '230', '300', '330', '400', '430', '500', '530', '600', '630', '700', '730', '800']
CONF_USER_COOL_MODE = ['off', '1', '2', '3', '4', '5', '6', '7', '8']
CONF_WEEK_PROGRAM = ['off', 'Program 1', 'Program 2', 'Program 3', 'Program 4']

nilan_ns = cg.esphome_ns.namespace('nilan')

CONFIG_SCHEMA = cv.All(
    select.SELECT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_NILAN_ID): cv.use_id(Nilan),
            #cv.Optional(CONF_OPTIONS): cv.All(
            #    cv.ensure_list(cv.string_strict), cv.Length(min=1)
            #),
            cv.Optional(CONF_USER_FUNC_MODE): ?????,
            cv.Optional(CONF_USER_TIME_MODE): ?????,
            cv.Optional(CONF_USER_COOL_MODE): ?????,
            cv.Optional(CONF_WEEK_PROGRAM): ?????,
            cv.Optional(CONF_OPTIMISTIC): cv.boolean,
            cv.Optional(CONF_SET_ACTION): automation.validate_automation(single=True),
            cv.Optional(CONF_INITIAL_OPTION): cv.one_of(*CONF_USER_FUNC_MODE_OPTIONS) # May need one like this for all the entities?
            #cv.Optional(CONF_RESTORE_VALUE): cv.boolean,
            #cv.Optional(CONF_NILAN_USER_FUNC_MODE): select.select_schema(),
        }
    ).extend(cv.polling_component_schema('60s')),
)

async def to_code(config):
    nilan = yield cg.get_variable(config[CONF_NILAN_ID])
    #var = cg.new_Pvariable(config[CONF_ID], nilan)
    #await cg.register_component(var, config)
    
    # not sure if this is correct?
    if CONF_USER_FUNC_MODE in config:
        conf = config[CONF_USER_FUNC_MODE]
        await select.register_select(var, config, options=CONF_USER_FUNC_MODE_OPTIONS)

    #if CONF_NILAN_USER_FUNC_MODE in config:
    #    conf = config[CONF_NILAN_USER_FUNC_MODE]
    #   sel = yield select.new_select(conf)
    #    cg.add(nilan.set_nilan_user_func_mode_select(sel)