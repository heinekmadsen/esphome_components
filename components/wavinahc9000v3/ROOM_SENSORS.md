# Use your own temperature sensor

Make a room follow any Home Assistant temperature entity instead of the Wavin
thermostat on the wall.

Everything is set up in the device YAML. Home Assistant needs no configuration.

---

## Setup

**1.** Have the component installed and talking to your controller — see
[INSTALLATION.md](INSTALLATION.md).

**2.** In your device YAML, describe each room:

```yaml
climate:
  - platform: wavinahc9000v3
    name: Bedroom
    channels: [1]

  - platform: wavinahc9000v3
    name: Living room
    channels: [3, 4]
```

**3.** Press **Install** in the ESPHome add-on.

Each room is now a thermostat in Home Assistant, working exactly like the Wavin
thermostat it sits on top of.

---

## Point a room at your own sensor

Find the sensor's name in **Developer tools → States**. It starts with
`sensor.` and its state is a plain number, e.g. `sensor.bedroom_temperature`.

Add one line to the room:

```yaml
  - platform: wavinahc9000v3
    name: Bedroom
    channels: [1]
    sensor: sensor.bedroom_temperature
```

Press **Install**. Done — the room now follows that sensor.

> `sensor:` must line up under `channels:`. YAML is fussy about indentation.

---

## Notes

- `channels:` are the output numbers on the Wavin controller. Use several for
  one room, e.g. `[3, 4]`.
- Up to 8 rooms.
- Set the temperature from the thermostat card. It survives restarts.
- To hand a room back to its wall thermostat, switch it off in Home Assistant
  or remove the `sensor:` line.

**Nothing can get stuck.** If the sensor is missing, misspelled or goes
unavailable, the room quietly behaves like the ordinary Wavin thermostat. It
never ends up without heating control.

| Problem | Fix |
|---|---|
| Card shows a temperature, but not your sensor's | `sensor:` line missing or name wrong |
| Shows *Unknown* just after installing | Wait a minute — Home Assistant is reconnecting |
| Red error on install | Check the indentation of `sensor:` |

---

## How it works

The controller has no way to be told "the room is 21.4 °C" — that reading can
only come from its own wall thermostat. So the setpoint is nudged instead:

```
setpoint written = wall thermostat reading + (your target − your sensor)
```

If your sensor says the room is 1 °C colder than the wall thermostat thinks, the
setpoint goes 1 °C higher. The Wavin keeps doing all the real control: its
hysteresis, valve timing, pump and frost protection are untouched. Only the
setpoint is ever written, and it is rate-limited to protect the controller's
EEPROM (about 6 writes an hour, not one per poll).

Wall thermostats stay installed and working. Nothing is deleted or overridden.

### Options

Set on the `wavinahc9000v3:` block, all optional:

| Key | Default | Meaning |
|---|---|---|
| `sensor_timeout` | `30min` | No reading for this long → use the wall thermostat |
| `min_write_interval` | `10min` | Minimum gap between setpoint writes |
| `setpoint_deadband` | `0.3` | Ignore changes smaller than this (°C) |
| `min_setpoint` / `max_setpoint` | `5` / `35` | Clamp on the written setpoint |

The room name and channels come from the YAML on every boot; the target
temperature and on/off state are kept on the device, so your setting survives a
reboot or a reflash.

Rooms can also be created at runtime instead of in the YAML — use `zone: N`
without `channels:`, plus the `zone_configure` / `zone_temperature` /
`zone_unavailable` / `zone_clear` services. Not needed for normal use.
