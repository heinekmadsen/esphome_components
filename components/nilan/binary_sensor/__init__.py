import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from .. import Nilan, CONF_NILAN_ID
from esphome.const import (
    CONF_ID,
    CONF_SENSOR,
    CONF_DEVICE_CLASS,
    CONF_ICON,
    UNIT_EMPTY,
    ICON_CHECK_CIRCLE_OUTLINE,
    DEVICE_CLASS_EMPTY
)
 
CONF_IS_SUMMER = "is_summer"
CONF_FILTER_OK = "filter_ok"
CONF_ON_OFF_STATE = "on_off_state"
CONF_BYPASS_ON_OFF = "bypass_on_off"
CONF_DOOR_OPEN = "door_open"

nilan_ns = cg.esphome_ns.namespace('nilan')
#NilanBinarySensor = nilan_ns.class_('NilanBinarySensor', binary_sensor.BinarySensor, cg.Component)
 
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_NILAN_ID): cv.use_id(Nilan),
    cv.Required(CONF_ON_OFF_STATE): binary_sensor.BINARY_SENSOR_SCHEMA.extend({
        cv.Optional(CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY): binary_sensor.device_class
    }),
    cv.Optional(CONF_IS_SUMMER): binary_sensor.BINARY_SENSOR_SCHEMA.extend({
        cv.Optional(CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY): binary_sensor.device_class
    }),
    cv.Optional(CONF_FILTER_OK): binary_sensor.BINARY_SENSOR_SCHEMA.extend({
        cv.Optional(CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY): binary_sensor.device_class
    }),
    cv.Optional(CONF_BYPASS_ON_OFF): binary_sensor.BINARY_SENSOR_SCHEMA.extend({
        cv.Optional(CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY): binary_sensor.device_class
    }),
    cv.Optional(CONF_DOOR_OPEN): binary_sensor.BINARY_SENSOR_SCHEMA.extend({
        cv.Optional(CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY): binary_sensor.device_class
    })
}).extend(cv.COMPONENT_SCHEMA)
 
def to_code(config):
    nilan = yield cg.get_variable(config[CONF_NILAN_ID])

    if CONF_IS_SUMMER in config:
        conf = config[CONF_IS_SUMMER]
        sens = yield binary_sensor.new_binary_sensor(conf)
        cg.add(nilan.set_is_summer_sensor(sens))
    if CONF_FILTER_OK in config:
        conf = config[CONF_FILTER_OK]        
        sens = yield binary_sensor.new_binary_sensor(conf)
        cg.add(nilan.set_filter_ok_sensor(sens))
    if CONF_ON_OFF_STATE in config:
        conf = config[CONF_ON_OFF_STATE]        
        sens = yield binary_sensor.new_binary_sensor(conf)
        cg.add(nilan.set_on_off_state_sensor(sens))
    if CONF_BYPASS_ON_OFF in config:
        conf = config[CONF_BYPASS_ON_OFF]        
        sens = yield binary_sensor.new_binary_sensor(conf)
        cg.add(nilan.set_bypass_on_off_sensor(sens))
    if CONF_DOOR_OPEN in config:
        conf = config[CONF_DOOR_OPEN]        
        sens = yield binary_sensor.new_binary_sensor(conf)
        cg.add(nilan.set_door_open_sensor(sens))