# Manual and advanced configuration

Normal installations should use **[README.md](README.md)** and
**[example_auto.yaml](example_auto.yaml)** with the companion Home Assistant
integration. This document is for tuning, diagnostics and direct service use.

## Component options

```yaml
auto_nilan:
  id: nilan
  uart_id: modbus_uart
  address: 30
  update_interval: 10s
  receive_timeout: 600ms
  inter_frame_delay: 4000
  # flow_control_pin: GPIO10
```

| Option | Default | Meaning |
|---|---|---|
| `id` | required | Component id used in lambdas and services |
| `uart_id` | required | UART bus connected to the RS-485 transceiver |
| `address` | `30` | Modbus slave address of the controller (1-247) |
| `update_interval` | `30s` | Poll interval. `10s` is recommended for Home Assistant |
| `receive_timeout` | `600ms` | Time to wait for a complete response frame |
| `inter_frame_delay` | `4000` | Minimum bus-idle time (microseconds) before a frame |
| `flow_control_pin` | none | DE/RE pin when the UART bus does not drive it. Prefer `uart: flow_control_pin` |

### UART

```yaml
uart:
  id: modbus_uart
  tx_pin: GPIO21
  rx_pin: GPIO20
  baud_rate: 19200
  stop_bits: 1
  parity: EVEN
  flow_control_pin: GPIO10   # Rev1 module; use GPIO3 on Rev2
```

The Nilan controllers use 19200 baud, 8 data bits, even parity and one stop bit.
The component is a Modbus RTU master; do not add a second master to the same bus.

Only set `flow_control_pin` in one place. If the UART is configured with
`flow_control_pin`, leave the component option out.

## Text sensors

The Home Assistant integration needs the `topology` and `values` text sensors.
`status` is optional but recommended for diagnostics.

```yaml
text_sensor:
  - platform: auto_nilan
    auto_nilan_id: nilan
    type: topology
    name: "Nilan Topology"
  - platform: auto_nilan
    auto_nilan_id: nilan
    type: values
    name: "Nilan Values"
  - platform: auto_nilan
    auto_nilan_id: nilan
    type: status
    name: "Nilan Status"
```

## Buttons

```yaml
button:
  - platform: auto_nilan
    auto_nilan_id: nilan
    action: commission
    name: "Nilan Commission"

  - platform: auto_nilan
    auto_nilan_id: nilan
    action: reset_alarm
    name: "Nilan Reset Alarm"

  - platform: auto_nilan
    auto_nilan_id: nilan
    action: reset_filter
    name: "Nilan Reset Filter"
```

| Action | Effect |
|---|---|
| `commission` | Re-detect the controller and rebuild the point list |
| `reset_alarm` | CTS602: write 255 to holding register 400. CTS400: write 1 to holding register 30 |
| `reset_filter` | Press the catalog `reset_filter` point (CTS400) |

The HA integration creates its own **Commission** and **Alarm reset** buttons, so
the YAML buttons are mainly useful when running the firmware without the
integration.

## Services

```yaml
api:
  services:
    - service: nilan_commission
      then:
        - lambda: 'id(nilan).commission();'

    - service: nilan_write_point
      variables:
        key: string
        value: float
      then:
        - lambda: |-
            std::string err;
            if (!id(nilan).write_point(key, value, &err))
              ESP_LOGW("nilan", "write_point failed: %s", err.c_str());

    - service: nilan_reset_alarm
      then:
        - lambda: 'id(nilan).reset_alarm();'
```

### Writing a point

Point keys come from the topology JSON. Examples:

```yaml
service: esphome.nilan_write_point
data:
  key: user_temp_set
  value: 21
```

```yaml
service: esphome.nilan_write_point
data:
  key: week_program
  value: 1
```

The firmware rejects unknown points, points that are read-only, points pruned
from this unit, and points that are not holding registers.

## Commissioning behavior

1. Detection reads the controller identification registers.
2. Capabilities are probed to select the relevant catalog subset.
3. Optional registers are read individually and removed when unsupported.
4. Block reads that return exception `0x02` are split, and only the registers
   that individually fail are pruned.
5. The topology text sensor is rebuilt and published.

Detection and commissioning retry every 60 seconds until a controller responds.
A manual **Commission** press starts a new attempt immediately.

The pruned list is not permanent: it is rebuilt on every boot/commission. A point
that works later will come back automatically.

## Diagnostics

The `status` text sensor reports:

| Field | Meaning |
|---|---|
| `online` | Last poll succeeded |
| `commissioned` | A controller was detected and the point list was built |
| `profile` / `model` / `model_name` | Detected controller |
| `caps` | Probed capabilities |
| `points` / `present` | Catalog candidates vs points that responded |
| `poll_ms` / `polls` | Poll duration and count |
| `tx` / `retries` / `failures` / `timeouts` / `crc_errors` / `exceptions` | Modbus counters |
| `last_error` | Last human-readable error |

Counters are cumulative. A burst of `failures` and `exceptions` during the first
commissioning is normal when unsupported registers are pruned; steady-state
operation should not increase them.

Set `logger: level: DEBUG` to log raw frames and per-register failures while
diagnosing wiring or unsupported points.

## Tuning

- **Timeouts on long cables:** increase `receive_timeout` to `800ms`.
- **Frequent CRC/framing errors:** check termination, shield grounding and
  cable length. The CTS602 pauses between bytes inside a response; do not
  reduce the receive timeout below 500 ms.
- **Slow updates:** lower `update_interval` only after confirming stable polls.
  A CTS602 poll takes roughly 0.7 s.
- **Multiple units:** each ESP32/controller pair needs its own address; change
  `address` if the controller is not at 30.

## Known limitations

- Validated on CTS602 hardware. CTS602 Light and CTS400 are implemented from the
  Nilan Modbus documentation and need field reports.
- The Modbus address is not auto-scanned; the Nilan default is 30.
- Baud rate/parity are fixed in YAML (19200 8E1 for all supported controllers).
- Removed points are pruned from the topology, but the Home Assistant integration
  may keep previously created entities until you delete or reload them.
- The component is a Modbus master. Do not connect another master to the same
  RS-485 bus.

## Updating

1. Update the external component source (branch, tag or commit).
2. Install the firmware again (OTA).
3. In Home Assistant, reload the Nilan Ventilation integration if the point list
   changed.
