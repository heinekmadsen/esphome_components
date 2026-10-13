import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID

from .. import MAX_ZONES, WavinAHC9000, WavinZoneClimate, WavinZoneThermostat

CONF_PARENT_ID = "wavinahc9000v3_id"
CONF_CHANNEL = "channel"
CONF_MEMBERS = "members"
CONF_SENSOR = "sensor"
CONF_ZONE = "zone"
CONF_VISUAL_MIN_TEMPERATURE = "visual_min_temperature"
CONF_VISUAL_MAX_TEMPERATURE = "visual_max_temperature"
CONF_STRICT_MODE_WRITES = "strict_mode_writes"
CONF_USE_FLOOR_TEMPERATURE = "use_floor_temperature"
# Resolved during validation, consumed by to_code. Not user-facing.
CONF_RESOLVED_ZONE = "resolved_zone"

# Zone slots are handed out in declaration order, so the YAML never has to
# mention a slot number. Explicit `zone:` entries reserve their slot, and a
# collision is reported rather than silently overwriting a room.
_used_zones: set[int] = set()


def _claim_zone(explicit=None):
    if explicit is not None:
        if explicit in _used_zones:
            raise cv.Invalid(f"zone {explicit} is already used by another climate block")
        _used_zones.add(explicit)
        return explicit
    for slot in range(1, MAX_ZONES + 1):
        if slot not in _used_zones:
            _used_zones.add(slot)
            return slot
    raise cv.Invalid(f"too many rooms: at most {MAX_ZONES} are supported")


CHANNEL_SCHEMA = climate.climate_schema(WavinZoneClimate).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Optional(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Optional(CONF_MEMBERS): cv.ensure_list(cv.int_range(min=1, max=16)),
        cv.Optional(CONF_STRICT_MODE_WRITES, default=False): cv.boolean,
        cv.Optional(CONF_USE_FLOOR_TEMPERATURE, default=False): cv.boolean,
    }
)

# Adding `sensor:` turns a climate into a room that follows that Home Assistant
# entity. The channels are named the same way as without it, with `channel:` or
# `members:`.
ROOM_SCHEMA = climate.climate_schema(WavinZoneThermostat).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Required(CONF_SENSOR): cv.entity_id,
        cv.Optional(CONF_CHANNEL): cv.int_range(min=1, max=16),
        cv.Optional(CONF_MEMBERS): cv.ensure_list(cv.int_range(min=1, max=16)),
        cv.Optional(CONF_ZONE): cv.int_range(min=1, max=MAX_ZONES),
        cv.Optional(CONF_VISUAL_MIN_TEMPERATURE, default=5.0): cv.float_range(min=0.0, max=30.0),
        cv.Optional(CONF_VISUAL_MAX_TEMPERATURE, default=35.0): cv.float_range(min=5.0, max=40.0),
    }
)

# Runtime-commissioned slot (advanced): no channels in the YAML.
ZONE_SCHEMA = climate.climate_schema(WavinZoneThermostat).extend(
    {
        cv.GenerateID(CONF_PARENT_ID): cv.use_id(WavinAHC9000),
        cv.Required(CONF_ZONE): cv.int_range(min=1, max=MAX_ZONES),
        cv.Optional(CONF_VISUAL_MIN_TEMPERATURE, default=5.0): cv.float_range(min=0.0, max=30.0),
        cv.Optional(CONF_VISUAL_MAX_TEMPERATURE, default=35.0): cv.float_range(min=5.0, max=40.0),
    }
)


def CONFIG_SCHEMA(config):
    """`sensor:` = a room, `zone:` alone = runtime slot, otherwise plain."""
    if not isinstance(config, dict):
        return CHANNEL_SCHEMA(config)

    is_room = CONF_SENSOR in config
    is_slot = CONF_ZONE in config and CONF_CHANNEL not in config and CONF_MEMBERS not in config

    if not is_room and not is_slot:
        return CHANNEL_SCHEMA(config)

    if is_room:
        if CONF_CHANNEL not in config and CONF_MEMBERS not in config:
            raise cv.Invalid("a room needs 'channel:' or 'members:' as well as 'sensor:'")
        if CONF_CHANNEL in config and CONF_MEMBERS in config:
            raise cv.Invalid("use either 'channel:' or 'members:', not both")
        if CONF_USE_FLOOR_TEMPERATURE in config:
            raise cv.Invalid(
                "'use_floor_temperature' cannot be combined with 'sensor:' - a room "
                "follows the sensor you name, not the floor probe"
            )

    out = (ROOM_SCHEMA if is_room else ZONE_SCHEMA)(config)
    # Claim the slot here rather than at codegen so a clash is reported by
    # `esphome config`, before anything is built.
    out[CONF_RESOLVED_ZONE] = _claim_zone(out.get(CONF_ZONE))
    return out


async def _setup_room(config, hub):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate.register_climate(var, config)
    # Must also be a Component, otherwise setup() never runs and the room is
    # never bound to its channels or its sensor.
    await cg.register_component(var, config)
    cg.add(var.set_parent(hub))

    zone = config[CONF_RESOLVED_ZONE]
    cg.add(var.set_zone(zone))
    cg.add(
        var.set_visual_range(
            config[CONF_VISUAL_MIN_TEMPERATURE], config[CONF_VISUAL_MAX_TEMPERATURE]
        )
    )

    channels = []
    if CONF_CHANNEL in config:
        channels = [config[CONF_CHANNEL]]
    elif CONF_MEMBERS in config:
        channels = config[CONF_MEMBERS]
    if channels:
        mask = 0
        for ch in channels:
            mask |= 1 << (ch - 1)
        cg.add(var.set_static_channels(mask))

    # Subscribe to the Home Assistant temperature entity directly over the
    # native API, so nothing has to be configured on the HA side.
    if CONF_SENSOR in config:
        cg.add(var.set_source_entity(config[CONF_SENSOR]))
        cg.add_define("USE_API_HOMEASSISTANT_STATES")
    cg.add(hub.add_zone_climate(zone, var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PARENT_ID])

    if CONF_RESOLVED_ZONE in config:
        await _setup_room(config, hub)
        return

    var = cg.new_Pvariable(config[CONF_ID])
    await climate.register_climate(var, config)
    cg.add(var.set_parent(hub))
    if CONF_CHANNEL in config:
        cg.add(var.set_single_channel(config[CONF_CHANNEL]))
        cg.add(hub.add_active_channel(config[CONF_CHANNEL]))
        if config[CONF_STRICT_MODE_WRITES]:
            cg.add(hub.set_strict_mode_write(config[CONF_CHANNEL], True))
        if config[CONF_USE_FLOOR_TEMPERATURE]:
            cg.add(var.set_use_floor_temperature(True))
        cg.add(hub.add_channel_climate(var))
    if CONF_MEMBERS in config:
        cg.add(var.set_members(config[CONF_MEMBERS]))
        for ch in config[CONF_MEMBERS]:
            cg.add(hub.add_active_channel(ch))
        cg.add(hub.add_group_climate(var))

