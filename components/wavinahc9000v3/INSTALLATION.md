# First setup

Getting from a wired-up ESP32 to working thermostats in Home Assistant.
For the full list of options and entity types, see [README.md](README.md).

## Before you start

- An ESP32 running ESPHome.
- An RS-485 adapter wired to the controller's A/B pair and to the ESP32. The
  examples use GPIO17 for TX and GPIO16 for RX.
- If your adapter has a DE/RE pin, note which GPIO it is on.

## 1. Flash a starting configuration

```yaml
esphome:
  name: wavin-node

esp32:
  board: esp32dev
  framework:
    type: esp-idf

external_components:
  - source:
      type: git
      url: https://github.com/heinekmadsen/esphome_components
      ref: "v1.0.1"
      path: components
    components: [wavinahc9000v3]

wifi:
  ssid: "<your-ssid>"
  password: "<your-password>"

logger:
  level: INFO

api:

ota:
  - platform: esphome

uart:
  id: uart_wavin
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 38400

wavinahc9000v3:
  id: wavin
  uart_id: uart_wavin
  update_interval: 5s
  # flow_control_pin: GPIO23        # if your adapter has a DE/RE pin
  # module: ustepper                # if you use the uStepper RS485 hat
  channel_01_friendly_name: "Bedroom"
  channel_02_friendly_name: "Living Room"

button:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Dump Wavin YAML"
```

Install it and leave the node running for 30-60 seconds.

## 2. Wait for discovery

Watch the log in the ESPHome dashboard. When each channel you have reports a
temperature, a setpoint and an action without repeated warnings, discovery has
enough data.

## 3. Ask the component for your YAML

Press the **Dump Wavin YAML** button in Home Assistant. The log fills with
entity blocks between two banner lines:

```
==================== Wavin YAML SUGGESTION BEGIN ====================
climate:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom"
    channel: 1

sensor:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom Battery"
    channel: 1
    type: battery
...
===================== Wavin YAML SUGGESTION END =====================
```

Instead of the button you can call `generate_yaml_suggestion()` from an API
service, or type `api.service dump_wavin_yaml` while attached with
`esphome logs`.

If something is missing - a comfort climate, a sensor - wait another minute and
press it again. The output always shows what has been polled so far.

## 4. Keep what you want

Paste the blocks you like into your configuration, rename them to taste, and
install again. Press the button any time your installation changes.

## 5. Optional: let a room follow your own sensor

Replace a channel's climate block with a room, naming a Home Assistant
temperature entity:

```yaml
climate:
  - platform: wavinahc9000v3
    wavinahc9000v3_id: wavin
    name: "Bedroom"
    channels: [1]
    sensor: sensor.bedroom_temperature
```

See [ROOM_SENSORS.md](ROOM_SENSORS.md).

## If it does not work

| Symptom | Try |
|---|---|
| No data at all | A and B swapped, wrong baud rate, or a missing direction pin |
| Nothing in the log after pressing the button | The node is offline, or the button block was not installed |
| Replies time out on a uStepper hat | Add `module: ustepper` |
| Bus warnings and retries | Check wiring and RS-485 termination; set `logger:` to DEBUG |
| A comfort climate is missing | The floor probe has not read a plausible value yet, above 1 °C and below 90 °C |
| Group names say `Zone G` | Give every member channel a `channel_NN_friendly_name` |

Discovery too slow? Set `poll_channels_per_cycle: 8` and `update_interval: 2s`
while setting up, then put them back.