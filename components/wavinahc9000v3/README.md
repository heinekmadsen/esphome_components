# Wavin AHC 9000 / Jablotron AC-116

ESPHome component for the Wavin AHC 9000 floor heating controller (sold as the
Jablotron AC-116), over RS-485.

- Up to 16 channels, discovered automatically
- One Home Assistant thermostat per room, single channel or several joined
- A room can follow any temperature entity you choose - see
  **[ROOM_SENSORS.md](ROOM_SENSORS.md)**
- Per-channel sensors: battery, air temperature, floor temperature, setpoint
- Child lock, setpoint numbers, repair button
- Prints a ready-to-paste YAML suggestion for your own installation

Setting one up for the first time? Start with
**[INSTALLATION.md](INSTALLATION.md)**. This page is the reference.

---

## 1. Wiring

RS-485 A/B from the controller to a TTL-to-RS-485 adapter, then to an ESP32.
Pins 16/17 are known good for the UART.

If your adapter needs direction control, add **one** of these to the
`wavinahc9000v3:` block:

| Option | Behaviour | Use when |
|---|---|---|
| `flow_control_pin` | HIGH only while transmitting | Most MAX3485 / 75176 boards. Prefer this. |
| `tx_enable_pin` | HIGH enables the driver, often left HIGH | Existing wiring that expects a static enable |

If the transceiver enables itself, leave both out.

Avoid the ESP32 strapping pins (GPIO0, 2, 4, 5, 12, 15); GPIO18/19/21/22/23/25/26/27/32/33 are safe.

Using the uStepper RS485 expansion? Add `module: ustepper` for longer bus guard
times, otherwise acknowledgements can arrive truncated.

---

## 2. Minimal configuration

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/heinekmadsen/esphome_components
      ref: "v1.0.1"
      path: components
    components: [wavinahc9000v3]

uart:
  id: uart_wavin
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 38400

wavinahc9000v3:
  id: wavin
  uart_id: uart_wavin
  update_interval: 5s
  channel_01_friendly_name: "Bedroom"
  channel_02_friendly_name: "Living Room"
  channel_03_friendly_name: "Kitchen"
```

Friendly names are optional and only used to label generated suggestions;
channels without one are called `Zone N`. A suggested group climate combines
its members' names: `A & B` for two, `A, B & C` for three or four, and
`First - Last` beyond that.

---

## 3. Climates

One thermostat in Home Assistant per channel, or one covering several.

```yaml
climate:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom"
    channel: 1

  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Living Room & Kitchen"
    members: [2, 3]
```

For a group, current temperature and setpoint are the average of its members,
and the action is *heating* if any member is heating.

| Option | Meaning |
|---|---|
| `channel` | Single channel, 1-16 |
| `members` | Several channels in one entity |
| `sensor` | Follow this Home Assistant temperature entity, see below |
| `use_floor_temperature` | Report the floor probe as the current temperature |
| `strict_mode_writes` | Marks the channel for strict baseline writes. Accepted and stored, but nothing reads the flag yet, so it currently changes nothing. |

### Following your own sensor

Add `sensor:` and the room follows that Home Assistant entity instead of the
Wavin wall thermostat:

```yaml
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom"
    channel: 1
    sensor: sensor.bedroom_temperature
```

Nothing is configured on the Home Assistant side, and the controller keeps
doing the actual control - the setpoint is biased by the difference between
your sensor and the wall thermostat. Full description in
**[ROOM_SENSORS.md](ROOM_SENSORS.md)**.

Such a room keeps its target temperature and on/off state on the device, so
both survive a reflash. Up to 16 of them. `sensor:` cannot be combined with
`use_floor_temperature:`.

### Comfort climate

With `use_floor_temperature: true` the entity reports the floor probe instead
of the air sensor, and exposes the controller's floor min/max as low/high
targets (5-35 °C, 0.5 °C steps, at least 1.0 °C apart).

```yaml
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bathroom Comfort"
    channel: 4
    use_floor_temperature: true
```

You can keep both an air-based and a floor-based entity for the same channel;
they share one setpoint. The entity stays unavailable until the probe reports a
plausible value (above 1 °C, below 90 °C).

---

## 4. Sensors

```yaml
sensor:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom Battery"
    channel: 1
    type: battery
```

| `type` | Reports |
|---|---|
| `battery` | Thermostat battery, 0-100 % |
| `temperature` | Air temperature |
| `floor_temperature` | Floor probe, unavailable until a plausible value is read |
| `floor_min_temperature` / `floor_max_temperature` | Floor limits |
| `comfort_setpoint` | Current comfort setpoint, read-only |

---

## 5. Setpoint numbers

Writable comfort and standby setpoints, 5-35 °C in 0.5 °C steps.

```yaml
number:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom Comfort Setpoint"
    channel: 1
    type: comfort      # or: standby
```

---

## 6. Child lock

Locks the buttons on the physical thermostat while Home Assistant can still
change the setpoint. ON locks, OFF unlocks.

```yaml
switch:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Office Lock"
    channel: 9
```

The switch shows the new state at once and the next poll confirms it. If it
flips back, the write failed - check the log at DEBUG level. If it is stuck
off, the channel has not finished discovery yet.

Before locking every thermostat, make sure you can still change setpoints from
Home Assistant.

---

## 7. Buttons

```yaml
button:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Dump Wavin YAML"        # prints the suggestion to the log

  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Repair Channel 3"
    channel: 3
    # extended: true               # also clear flags that can lock the keypad
    # aggressive: true
    # normalize: true
    # normalize_off: true
```

---

## 8. Letting the component write your YAML

The component can draft entity blocks for the channels it finds.

1. Flash the minimal configuration above with `logger:` at INFO.
2. Wait 30-60 seconds for discovery to settle.
3. Press the dump button, or call the service:

```yaml
api:
  services:
    - service: dump_wavin_yaml
      then:
        - lambda: 'id(wavin).generate_yaml_suggestion();'
```

4. Copy what you want out of the log, between:

```
==================== Wavin YAML SUGGESTION BEGIN ====================
===================== Wavin YAML SUGGESTION END =====================
```

5. Paste it into your node YAML and install.

Channels that end up in a group are included as single climates too, but
commented out, so you can uncomment them if you would rather manage them
separately. Run it again at any time - the output always reflects the current
discovery state, so a floor probe or thermostat that had not reported yet will
appear in a later run.

---

## 9. Hub options

All optional except `uart_id`.

| Option | Default | Meaning |
|---|---|---|
| `uart_id` | - | The UART to use. Required. |
| `update_interval` | `5s` | Poll interval |
| `poll_channels_per_cycle` | `2` | Channels read per cycle; raise to discover faster |
| `flow_control_pin` / `tx_enable_pin` | - | RS-485 direction control, see section 1 |
| `module` | `default` | `ustepper` for slightly longer transceiver turnaround |
| `inter_frame_delay_us` | from `module` | Silence enforced between transactions. Raising it did not help on a tested AHC-9000; try it only if the controller ignores requests. |
| `receive_timeout_ms` | `200` | How long to wait for a reply to *start*. Measured latency on an AHC-9000 is 3-39 ms. |
| `temp_divisor` | `10.0` | Raw-to-°C scaling |
| `allow_mode_writes` | `true` | Allow writing the channel mode |
| `keep_standby_alive` | `false` | Reassert OFF periodically if channels drift back to heat |
| `standby_keepalive_interval` | `180s` | How often; `0s` disables |
| `channel_NN_friendly_name` | - | Label for channel NN, `01`-`16` |

Only used by rooms that follow their own sensor:

| Option | Default | Meaning |
|---|---|---|
| `sensor_timeout` | `30min` | No reading for this long, fall back to the wall thermostat |
| `min_write_interval` | `10min` | Minimum gap between setpoint writes |
| `setpoint_deadband` | `0.3` | Ignore changes smaller than this, in °C |
| `min_setpoint` / `max_setpoint` | `5` / `35` | Clamp on the written setpoint |

---

## 10. Bus health

Every five minutes the component logs one line summarising the RS-485 link:

```
bus: tx=213 retry=9 fail=0 timeout=9 crc=0 resync=0 drop_pre=0 drop_post=0
     reply_us min=3023 avg=16237 max=38642
```

| Field | Meaning |
|---|---|
| `tx` | Transactions attempted |
| `retry` | Attempts beyond the first |
| `fail` | Transactions that gave up after every retry |
| `timeout` | Attempts where no usable frame arrived |
| `crc` | Frames that arrived damaged |
| `resync` | Bytes skipped to find a frame header - noise on the line |
| `drop_pre` / `drop_post` | Stale bytes discarded before transmitting, and echo discarded after |
| `reply_us` | How long the controller took to start answering |

A healthy bus shows `fail=0`, `crc=0` and `resync=0`, with `retry` a small
percentage of `tx`. Persistent `crc` or `resync` means electrical noise -
check termination and cable routing. Rising `timeout` with no `crc` means the
controller is not answering: usually wiring, the direction pin, or a bus the
master is still driving.

## 11. Troubleshooting

| Symptom | Try |
|---|---|
| No entities, no data | Check A/B are not swapped, baud rate 38400, and the direction pin |
| Values appear then stop | Bus contention - prefer `flow_control_pin` over `tx_enable_pin` |
| Retries climbing, `crc` or `resync` non-zero | Electrical noise; check termination and routing |
| Retries climbing, only `timeout` | Raise `receive_timeout_ms` if `reply_us max` is near it |
| A channel never shows up | It has no paired thermostat, or discovery has not reached it yet |
| Floor sensor unavailable | No plausible reading yet; it needs above 1 °C and below 90 °C |
| Child lock flips back | The write failed; look for the masked write result at DEBUG |

Useful while bringing a system up: `logger:` at DEBUG, and a higher
`poll_channels_per_cycle` to speed up discovery - put it back afterwards.

---

## Project status

A fast-moving, reverse-engineered community project.

- Register meanings may change as more controllers are tested.
- Floor and comfort logic, group naming and friendly-name composition are
  pragmatic rather than final.
- Refactors favour clarity and onboarding over keeping generated suggestions
  byte-identical.
- If you pin this for production, read the diff before updating, especially
  around write behaviour.

Bug reports and captures are welcome - please include DEBUG logs, how many
channels you have, and whether floor probes are present.

## Disclaimer

Community integration, use at your own risk. Not affiliated with Wavin or
Jablotron.