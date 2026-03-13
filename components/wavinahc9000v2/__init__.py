import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID


def _register_yaml_representers() -> None:
    """Ensure ESPHome can dump configs containing codegen objects.

    ESPHome 2026.1.x may place codegen objects (e.g. MockObj enums like
    modbus_controller::ModbusRegisterType::CUSTOM) into the validated config.
    During compile ESPHome computes a config hash by YAML-dumping the config;
    PyYAML can't serialize these objects by default.
    """

    try:
        import yaml
        from esphome import cpp_generator
        from esphome import yaml_util

        dumper_cls = getattr(yaml_util, "ESPHomeDumper", None)
        if dumper_cls is None:
            return

        def _represent_codegen_obj(dumper, value):
            return dumper.represent_scalar("tag:yaml.org,2002:str", str(value))

        yaml.add_representer(cpp_generator.MockObj, _represent_codegen_obj, Dumper=dumper_cls)
    except Exception:
        # Best-effort only; should never block config validation.
        return


_register_yaml_representers()

wavinahc9000v2_ns = cg.esphome_ns.namespace('wavinahc9000v2')
Wavinahc9000v2 = wavinahc9000v2_ns.class_('Wavinahc9000v2', cg.Component)

CONF_WAVINAHC9000v2_ID = 'wavinahc9000v2_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Wavinahc9000v2),   
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
