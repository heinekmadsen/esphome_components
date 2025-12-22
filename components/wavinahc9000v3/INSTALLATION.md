# Installation & Onboarding Guide

This guide explains how to connect the Wavin AHC 9000 (Jablotron AC‑116) external component, capture its automatically generated YAML suggestion from the ESPHome log, and fold the results into your production configuration.

## Prerequisites
- ESPHome 1.21+ (or any recent release) running on an ESP32 board.
- RS‑485 adapter wired to the controller (A/B twisted pair) and to ESP32 UART pins (examples below use GPIO16 = RX, GPIO17 = TX).
- Optional: floor probes if you plan to expose comfort climates.
- Optional: a direction-control GPIO (DE/RE). The integration supports either `flow_control_pin` (preferred) or `tx_enable_pin` if your breakout keeps the driver enabled.

### Direction Control Quick Reference
| Option | Behavior | When to use |
|--------|----------|-------------|
| `flow_control_pin` | Pulsed HIGH only while transmitting, LOW for receive. Unified DE/RE control. | Recommended for most MAX3485/75176 boards. |
| `tx_enable_pin` | HIGH enables the driver (often held HIGH). | Use only if you already wired a dedicated enable pin. |

Provide at most one of the two. If your interface auto-manages half-duplex, you can omit both entirely.

### Module Profiles
- Default (implicit): Works for most MAX3485/75176 breakouts.
- `module: ustepper`: Adds additional pre/post transmit guard delays and clears the RX buffer before writes. Use this when you interface the controller through the uStepper RS485 add-on – it prevents truncated acknowledgements on ESP32-C3 boards.

## Step 1: Base ESPHome Config
Start with a lean configuration that includes logging and a helper service to trigger the YAML dump:

```yaml
esphome:
  name: wavin-node

esp32:
  board: esp32dev
  framework:
    type: esp-idf

external_components:
  - source: github://heinekmadsen/esphome_wavinahc9000v3
    components: [wavin_ahc9000]

logger:
  level: INFO  # temporarily raise to DEBUG if you need extra polling detail

uart:
  id: uart_wavin
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 38400
  parity: NONE
  stop_bits: 1

wavin_ahc9000:
  id: wavin
  uart_id: uart_wavin
  update_interval: 5s
  # module: ustepper  # enable extended RS485 guard timings for uStepper RS485 hats
  # Optional direction control (choose at most one)
  # flow_control_pin: GPIO23
  # tx_enable_pin: GPIO23
  # Optional discovery speed-up (briefly uncomment)
  # poll_channels_per_cycle: 8
  # update_interval: 2s
  # Optional friendly names (used in logged suggestions)
  channel_01_friendly_name: "Bedroom"
  channel_02_friendly_name: "Living Room"

api:
  services:
    - service: dump_wavin_yaml
      then:
        - lambda: 'id(wavin).generate_yaml_suggestion();'

wifi:
  ssid: "<your-ssid>"
  password: "<your-password>"

ota:
```

Upload this firmware. Leave the node running for roughly 30–60 seconds so the integration can discover channels and read initial register data.

## Step 2: Let Discovery Settle
Monitor the logs (`esphome logs wavin-node` or the ESPHome dashboard). Once each active channel reports current temperature, setpoint, and action without repeated warnings, the hub has enough data to build a meaningful suggestion.

## Step 3: Trigger the YAML Suggestion Dump
Call the helper service you added above:

- Home Assistant → Developer Tools → Services → `esphome.wavin_node_dump_wavin_yaml` (adjust for your node name), or
- ESPHome CLI: `esphome service wavin-node dump_wavin_yaml`.
- Prefer a dashboard button? Add this and press it from the Lovelace UI:

```yaml
button:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Dump Wavin YAML"
```

The ESPHome log prints the suggestion between banner lines, for example:

```
==================== Wavin YAML SUGGESTION BEGIN ====================
climate:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom"
    channel: 1

sensor:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom Battery"
    channel: 1
    type: battery

switch:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom Child Lock"
    channel: 1
...
===================== Wavin YAML SUGGESTION END =====================
```

Friendly names feed into the generated entity titles. Channels that belong to a grouped climate still appear as single climates, but they remain commented in the output so you can re-enable them later if you prefer individual control.

If the output looks incomplete (for example, missing comfort climates or battery sensors), wait another minute and call the service again. The suggestion reflects whatever the integration has most recently polled.

## Step 4: Copy Entities into Your Production Config
Copy the sections you care about—climates, sensors, child-lock switches—into your node YAML. Adjust the `name:` fields to taste. There is no onboarding package to disable afterward; once you have pasted the entities you can either keep the helper service (handy for future regeneration) or remove it.

Recompile and upload the updated configuration. Your device now exposes the selected entities with no extra scaffolding.

## Regenerating Later
Need to add a room or rename a zone? Just call the `dump_wavin_yaml` service again. The logged output always reflects the live state, so you can copy incremental changes without juggling packages or templates.

## Friendly Name Tips
- Provide whichever `channel_XX_friendly_name` keys you want; any missing entries fall back to `Zone N`.
- Group climate names combine member names (`A & B`, `A, B & C`, or `A – D`). If a member lacks a friendly name, a generic `Zone G` label is used.

## Troubleshooting
| Symptom | Suggested Action |
|---------|------------------|
| No banner appears after calling the service | Confirm the helper service exists, the node is online, and the ESPHome log shows no errors. |
| Suggestion missing comfort climates | The floor probe has not produced a plausible value (>1 °C and <90 °C) yet. Wait for additional polling and trigger the dump again. |
| Group names look generic (`Zone G`) | Add friendly names for each member channel and re-run the service. |
| Logged YAML still shows commented single climates | That is intentional; uncomment any channel you want to manage individually even if it participates in a group climate. |
| Bus warnings or repeated retries | Re-check wiring, RS‑485 termination, and (if used) direction-control GPIO. Temporarily set `logger:` to DEBUG for deeper insight. |
| Missing replies on uStepper hardware | Add `module: ustepper` under `wavin_ahc9000:` to slow the RS485 turnaround timing. |

## Optional: Quick Console Trigger
When connected via `esphome logs`, you can trigger the dump interactively:

```
> api.service dump_wavin_yaml
```

The same banners appear immediately in the console.

---

This lean workflow keeps your runtime configuration minimal while still letting the component guide you through building accurate climate and sensor blocks whenever your installation changes.
