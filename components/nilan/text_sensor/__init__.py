import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from .. import Nilan, CONF_NILAN_ID
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_ICON,
    CONF_DEVICE_CLASS
)
 
CONF_OPERATION_MODE = "operation_mode"
CONF_CONTROL_STATE = "control_state"
CONF_VERSION_INFO = "version_info"
CONF_DISPLAY_LINE1 = "display_line1"
CONF_DISPLAY_LINE2 = "display_line2"

nilan_ns = cg.esphome_ns.namespace('nilan')
 
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_NILAN_ID): cv.use_id(Nilan),
    cv.Optional(CONF_OPERATION_MODE): text_sensor.TEXT_SENSOR_SCHEMA.extend({cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}),
    cv.Optional(CONF_CONTROL_STATE): text_sensor.TEXT_SENSOR_SCHEMA.extend({cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}),
    cv.Optional(CONF_VERSION_INFO): text_sensor.TEXT_SENSOR_SCHEMA.extend({cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}),
    cv.Optional(CONF_DISPLAY_LINE1): text_sensor.TEXT_SENSOR_SCHEMA.extend({cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}),
    cv.Optional(CONF_DISPLAY_LINE2): text_sensor.TEXT_SENSOR_SCHEMA.extend({cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):    
    nilan = yield cg.get_variable(config[CONF_NILAN_ID])

    if CONF_OPERATION_MODE in config:
        conf = config[CONF_OPERATION_MODE]
        var = cg.new_Pvariable(conf[CONF_ID])
        yield text_sensor.register_text_sensor(var, conf)
        cg.add(nilan.set_operation_mode_sensor(var))
    if CONF_CONTROL_STATE in config:
        conf = config[CONF_CONTROL_STATE]
        var = cg.new_Pvariable(conf[CONF_ID])
        yield text_sensor.register_text_sensor(var, conf)
        cg.add(nilan.set_control_state_sensor(var))
    if CONF_VERSION_INFO in config:
        conf = config[CONF_VERSION_INFO]
        var = cg.new_Pvariable(conf[CONF_ID])
        yield text_sensor.register_text_sensor(var, conf)
        cg.add(nilan.set_version_info_sensor(var))
    if CONF_DISPLAY_LINE1 in config:
        conf = config[CONF_DISPLAY_LINE1]
        var = cg.new_Pvariable(conf[CONF_ID])
        yield text_sensor.register_text_sensor(var, conf)
        cg.add(nilan.set_display_line1_sensor(var))
    if CONF_DISPLAY_LINE2 in config:
        conf = config[CONF_DISPLAY_LINE2]
        var = cg.new_Pvariable(conf[CONF_ID])
        yield text_sensor.register_text_sensor(var, conf)
        cg.add(nilan.set_display_line2_sensor(var))