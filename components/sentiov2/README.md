# Wavin Sentio - ESPHome V2

Automatic room discovery for a Wavin Sentio heating controller, with a topology
text sensor and services for the companion Home Assistant integration:

**[Wavin Floor Heating integration](https://github.com/nic6911/ha-wavin-floor-heating)**

The integration supports both the Wavin AHC9000 (`wavinahc9000v4`) and Wavin
Sentio (`sentiov2`). The firmware components stay separate.

## What it does

- Scans Sentio rooms 1-16 over Modbus RTU.
- Reads room type, current temperature, setpoint, humidity, dew point,
  heating/cooling state, lock state and warning flags.
- Publishes `Wavin Sentio Topology` JSON for Home Assistant.
- Exposes services for setpoint, on/off, child lock, vacation mode and manual
  topology refresh.
- Filters invalid Modbus readings (`0`, `327.67`, out-of-range values) so a
  transient bus error does not create a false 0 °C / 0 % spike in Home
  Assistant.

## Combined rooms

Some Sentio installations combine two physical channels into one thermostat
zone. The controller can expose the writable thermostat in one room object and
the temperature/humidity sensor in the next room object. `sentiov2` detects
this pattern and publishes one zone: the control room keeps its setpoint and
lock state, while the adjacent sensor-only room supplies current temperature,
humidity and heating state.

For example, a sensor-only room 10 immediately before a control-only room 11
becomes one zone with `sensor_room: 10`. Physical output numbers are separate
from Modbus room numbers, so output 6+10 combined does not mean room 6+10.

## Hardware'

Tested with the ESP32 Modbus Module. For the Rev2 module:

- UART TX: `GPIO21`
- UART RX: `GPIO20`
- RS-485 flow control: `GPIO3`
- Baud rate: `19200`

For Rev1, use the same configuration but set `flow_control_pin: GPIO10`.

## Example

Use **[example.yaml](example.yaml)**. It contains the full ESPHome
configuration, the topology text sensor and the API services.

The example is pinned to `1.0.3-stable`.

## Services

- `sentio_zone_set_target` - `zone`, `temperature`
- `sentio_zone_set_mode` - `zone`, `turn_off`
- `sentio_zone_set_child_lock` - `zone`, `enabled`
- `sentio_zone_set_vacation` - `enabled`
- `sentio_zone_set_sync` - `enabled`
- `sentio_zone_discover`

## Home Assistant

Install the Wavin Floor Heating integration, then add the discovered module.
The integration creates one Home Assistant device per room with a thermostat,
temperature, humidity, dew point, floor temperature, child lock and low-battery
indicator where the controller provides them.

If the Sentio controller does not expose room names over Modbus, devices are
created as `Room 1`, `Room 2`, and so on. You can rename them in Home
Assistant.
