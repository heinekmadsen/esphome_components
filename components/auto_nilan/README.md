# Nilan Ventilation - ESPHome

ESPHome firmware for Nilan ventilation units with Modbus RTU (RS-485):

- **CTS602** (full controller)
- **CTS602 Light** (same register map as CTS602; unsupported points are pruned)
- **CTS400**

The component detects the connected controller, probes its capabilities, builds
the point list that the unit actually supports, and publishes JSON topology,
values and status. It is designed for the companion Home Assistant integration:

**[Nilan Ventilation integration](https://github.com/nic6911/ha-nilan-ventilation)**

[![Open in Marketplace](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=nic6911&repository=ha-nilan-ventilation&category=integration)

No MQTT. No hand-written YAML entities. You do not need to know the model or the
register list in advance.

> **Status:** this component currently lives on the
> `feature/nilan_auto_discovery` branch of the
> [nic6911/esphome_components](https://github.com/nic6911/esphome_components/tree/feature/nilan_auto_discovery)
> fork while it is being tested. Point `ref:` at that branch until it is merged
> upstream.

---

## Start here

### 1. Wire the ESP32

Use a Nilan Modbus module or any ESP32 board with a 3.3 V RS-485 transceiver.

| Signal | Default |
|---|---|
| UART TX | `GPIO21` |
| UART RX | `GPIO20` |
| RS-485 direction (DE/RE) | `GPIO10` on Rev1, `GPIO3` on Rev2 |
| Baud rate | `19200`, 8 data bits, even parity, 1 stop bit |
| Modbus address | `30` |

Connect A/B to the Nilan Modbus terminals. The RJ45 pinout and board photos are
in the [ESP32 Modbus Module repository](https://github.com/nic6911/ESP32_Modbus_Module).

### 2. Flash the firmware

Ready-made YAML for the ESP32 Modbus Module:

- **Rev1 (direction pin GPIO10):**
  [nilan_ventilation_auto.yaml](https://github.com/nic6911/ESP32_Modbus_Module/blob/main/esphome_example_code/rev1/nilan_ventilation_auto.yaml)
- **Rev2 (direction pin GPIO3):**
  [nilan_ventilation_auto.yaml](https://github.com/nic6911/ESP32_Modbus_Module/blob/main/esphome_example_code/rev2/nilan_ventilation_auto.yaml)

Or start from **[example_auto.yaml](example_auto.yaml)** in this folder:

1. Open ESPHome and create a new device.
2. Paste the YAML.
3. Set the Wi-Fi credentials in `secrets.yaml`.
4. Change `flow_control_pin` for your board.
5. Install it to the ESP32.

The firmware starts commissioning immediately. It retries every 60 seconds until
a controller is found.

### 3. Install the Home Assistant integration

1. Go to **Settings -> Marketplace** in Home Assistant.
2. Select **Add from link** and enter:

   ```text
   https://github.com/nic6911/ha-nilan-ventilation
   ```

3. Choose **Integration** if asked, install **Nilan Ventilation**, and restart
   Home Assistant if prompted.
4. Home Assistant discovers the ESPHome device automatically. Select
   **Configure** on the discovered **Nilan Ventilation** card and confirm. No IP
   address is needed.

If discovery does not appear, add the integration manually and enter the ESP32
host name or IP address. Leave the API key empty unless you enabled ESPHome API
encryption.

---

## What you get

The integration creates entities from the firmware topology, so the entity list
matches the ventilation unit instead of a fixed YAML list:

- Temperature, humidity, CO2, pressure, capacity and status sensors
- Setpoints, timers and other writable numeric points
- Operating mode, fan step and other select entities
- Switches for on/off functions
- User-function buttons, plus **Commission** and **Alarm reset** actions
- One climate entity for CTS602 units (mode, fan step, current temperature)
- Diagnostic status sensors for poll time, retries, failures, timeouts, CRC
  errors and Modbus exceptions

Around 120 entities are typical for a CTS602; CTS400 exposes fewer points.

## Automatic detection

Detection runs on boot and whenever commissioning is requested:

| Controller | Test |
|---|---|
| CTS602 / CTS602 Light | Input register 0 = bus version 1-199, holding register 1000 = machine type <= 40 |
| CTS400 | Input register 24 = extract air <= 1000, holding register 37 = room setpoint <= 400 |

After detection the component probes capabilities (RH, CO2, VOC, cooling,
heating, hot water, compressor, bypass, central heat, preheat, expansion,
pressure) and filters the catalog (196 CTS602 points, 87 CTS400 points).

## Automatic commissioning

Commissioning is not a one-time setup step you have to trigger:

- Runs automatically on first boot.
- Retries every 60 seconds until a controller answers.
- Probes optional registers individually.
- Drops registers that the controller reports as illegal (Modbus exception
  `0x02`) instead of failing the whole poll.
- Rebuilds the topology when points are added or removed.

You can force a new scan with the **Nilan Commission** button in Home Assistant,
the `nilan_commission` ESPHome service, or by restarting the ESP32.

## Writing values

Writes use function code `0x10` (write multiple registers) on CTS602 and `0x06`
(write single register) on CTS400. Writes are rejected for points that are not
writable or not present on the connected unit.

From Home Assistant, just use the number/select/switch/climate entities. For
advanced use, the `nilan_write_point` service accepts a point key and a value.

## Topology JSON

The three text sensors are the interface used by the integration:

| Text sensor | Content |
|---|---|
| `topology` | Profile, model, capabilities and all points (key, name, register, scale, min/max, options) |
| `values` | Current values for all present points |
| `status` | Online, commissioned, poll timing and Modbus counters |

Keep the text sensors in the YAML. The integration needs at least `topology`
and `values`; `status` is used for diagnostics.

## Troubleshooting

| Problem | Try |
|---|---|
| No controller detected | Check A/B wiring, 19200 8E1, address 30, and that the Nilan unit has Modbus enabled |
| Values stay `unknown` | Open the ESPHome log and look for `no Nilan controller detected` or Modbus exceptions |
| Some entities are missing | The unit does not implement those registers; commissioning pruned them. That is expected |
| Writes do not change the unit | Use the HA entities; for direct writes check that the point is writable in the topology JSON |
| Frequent timeouts/CRC errors | Check termination, cable length, shield/ground, and that only one master is on the bus |
| Device offline | Check Wi-Fi and the fallback hotspot; the module restarts into `Nilan Fallback Hotspot` |

Set the logger to `DEBUG` temporarily to see frame-level Modbus diagnostics.

## Advanced

See **[MANUAL.md](MANUAL.md)** for:

- Every YAML option
- Text sensor and button configuration
- Services and manual point writes
- Tuning receive timeout and inter-frame delay
- Reset filter action for CTS400
- Known limitations

## Firmware family

- `auto_nilan` - automatic detection + Home Assistant integration (this folder)
- Legacy manual Nilan YAML examples - still available in the ESP32 Modbus Module
  repository, but not required for new installations

## Compatibility

- ESPHome 2026.10 or newer
- Home Assistant 2026.2 or newer (tested on 2026.11)
- Nilan CTS602, CTS602 Light, CTS400 with Modbus RTU
- Companion integration `ha-nilan-ventilation` 0.2.0 or newer

## Development

The component is currently validated on a CTS602 (VPL 10 without cooling) at
19200 8E1. CTS602 Light and CTS400 support is implemented from the Nilan Modbus
documentation; please report registers that are pruned or values that look
wrong on those units.

Issue tracker:
<https://github.com/nic6911/esphome_components/issues>
