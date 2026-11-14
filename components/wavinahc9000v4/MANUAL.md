# Manual and advanced configuration

Normal installations should use the automatic zone mode from
**[README.md](README.md)** and the companion Home Assistant integration.

Use this document only if you want to define entities yourself in YAML.

## Manual zones

Turn automatic zones off:

```yaml
wavinahc9000v4:
  id: wavin
  uart_id: uart_wavin
  auto_zones: false
```

Then create climates and sensors per channel:

```yaml
climate:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Bedroom"
    channel: 1

  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Living room & kitchen"
    members: [2, 3]

sensor:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Bedroom battery"
    channel: 1
    type: battery

  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Bedroom temperature"
    channel: 1
    type: temperature
```

## Climate options

| Option | Meaning |
|---|---|
| `channel` | One AHC channel, 1–16 |
| `members` | Several channels in one climate |
| `sensor` | Follow a Home Assistant temperature entity |
| `use_floor_temperature` | Use the floor probe as current temperature |

### External room sensor

With manual zones only, a climate can follow a Home Assistant temperature
sensor:

```yaml
climate:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Bedroom"
    channel: 1
    sensor: sensor.bedroom_temperature
```

The AHC controller keeps doing the real heating control. The integration
adjusts the written setpoint to compensate for the difference between the
wall thermostat and your sensor. If the sensor disappears, control returns to
the wall thermostat automatically.

### Floor probe climate

```yaml
climate:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Bathroom floor"
    channel: 4
    use_floor_temperature: true
```

The entity appears when the floor probe reports a plausible temperature.

## Sensors

| `type` | Reports |
|---|---|
| `battery` | Thermostat battery, 0–100 % |
| `temperature` | Air temperature |
| `floor_temperature` | Floor probe |
| `floor_min_temperature` | Floor minimum limit |
| `floor_max_temperature` | Floor maximum limit |
| `comfort_setpoint` | Comfort setpoint, read-only |

## Child lock

```yaml
switch:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Office lock"
    channel: 9
```

ON locks the physical thermostat buttons. OFF unlocks them.

## Setpoint numbers

```yaml
number:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Bedroom comfort setpoint"
    channel: 1
    type: comfort
```

Use `type: standby` for the standby setpoint.

## Buttons

```yaml
button:
  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Dump Wavin YAML"

  - platform: wavinahc9000v4
    wavinahc9000v4_id: wavin
    name: "Repair channel 3"
    channel: 3
```

`Dump Wavin YAML` prints ready-made manual YAML to the ESPHome log.

## Hub options

| Option | Default | Meaning |
|---|---|---|
| `update_interval` | `5s` | Poll interval |
| `poll_channels_per_cycle` | `2` | Channels advanced per poll |
| `flow_control_pin` | none | RS-485 DE/RE pin |
| `tx_enable_pin` | none | Legacy driver-enable pin |
| `module` | `default` | Set to `ustepper` for uStepper RS485 hardware |
| `auto_zones` | `true` | Automatic zone mode and HA integration |
| `auto_zone_scan_interval` | `30s` | How often AHC zones are checked |
| `allow_mode_writes` | `true` | Allow Home Assistant to change heat/off |
| `keep_standby_alive` | `false` | Periodically reassert OFF |
| `sensor_timeout` | `30min` | External sensor timeout |
| `min_write_interval` | `10min` | Minimum setpoint write gap |
| `setpoint_deadband` | `0.3` | Ignore smaller setpoint changes |
| `min_setpoint` / `max_setpoint` | `5` / `35` | Setpoint limits |

## Wiring notes

- RS-485 A/B from the controller to the ESP32 adapter.
- Preferred direction control: `flow_control_pin`.
- Avoid ESP32 strapping pins for direction control: GPIO0, 2, 4, 5, 12, 15.
- For the uStepper RS485 hat, add `module: ustepper`.

## Reference

The AHC9000 register specification is included as
`Wavin Modbus specification AHC9000 direkte.pdf`.
