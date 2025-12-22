# ESPHome Component: Wavin AHC 9000 / Jablotron AC-116 (v3 restart)

Integrates the Wavin AHC 9000 (a.k.a. Jablotron AC-116) floor heating controller via its RS‑485 protocol (custom function codes 0x43 / 0x44 / 0x45). Provides auto‑discovery assisted YAML generation, single & grouped climate entities, comfort (floor‑temperature based) climates, and optional per‑channel sensors.

## Key Features

| Feature | Status | Notes |
|---------|--------|-------|
| Up to 16 channels | ✅ | Automatically discovered through staged polling |
| Single channel climates | ✅ | One climate per active channel |
| Group climates | ✅ | Channels sharing same primary element grouped; composite name logic |
| Comfort climates | ✅ | Uses floor temperature as current temp when floor probe detected |
| Battery sensors | ✅ | Per channel (0–100%) |
| Temperature sensors | ✅ | Per channel ambient temperature |
| Floor probe detection | ✅ | Auto-detects plausible floor sensor (>1°C & <90°C) |
| Friendly names per channel | ✅ | `channel_XX_friendly_name` config; used in logged YAML suggestions |
| YAML suggestion (log output) | ✅ | Call `generate_yaml_suggestion()` to print entity templates |
| Commented single climates for grouped members | ✅ | Keeps originals (commented) for reference |
| Robust retry & polling pacing | ✅ | 2-attempt read/write retry logic |
| Child lock (per channel) | ✅ | Exposed as switch; toggles controller LOCK bit |

## Project Status / Vibe-Coding Disclaimer
This is a "vibe-coding" / fast-iteration community project:
* Reverse-engineered protocol pieces may evolve; some registers/assumptions could change as more hubs are tested.
* Logged YAML suggestion format is considered stable enough for use, but minor cosmetic tweaks (indentation, naming heuristics, commenting strategy) can still occur.
* Floor/comfort logic, group naming rules, and friendly-name composition are pragmatic rather than final standards.
* Expect occasional refactors prioritizing clarity and onboarding experience over strict backward compatibility of generated suggestions.
* If you pin this as a GitHub external component for production, review diffs before updating — especially around write behaviors.

Contributions (bug reports, captures, PRs) are welcome. Please include firmware logs (DEBUG level), channel counts, and whether floor probes are present to help refine heuristics.

## Quick Start (Final Config Example)
### TL;DR Workflow
1. Flash a standard config (see example below) with ESPHome logging enabled.
2. Let the hub poll for ~30–60 seconds until channel data looks stable.
3. Trigger `generate_yaml_suggestion()` (e.g. via an ESPHome API service) to print the recommended YAML to the log.
4. Copy the logged entity blocks into your permanent node YAML and adjust names as needed.
5. Recompile & upload the updated firmware.

Flowchart (Mermaid):
```mermaid
flowchart TD
  A[Start / Flash Config] --> B{Discovery Stable?}
  B -- No --> A
  B -- Yes --> C[Trigger generate_yaml_suggestion()]
  C --> D[Copy YAML from ESPHome Log]
  D --> E[Update Node Configuration]
  E --> F{Need Updates Later?}
  F -- Yes --> C
  F -- No --> G[Done]
```

Static diagram (SVG):
![Workflow Diagram](docs/workflow_diagram.svg)

```yaml
external_components:
  - source: github://heinekmadsen/esphome_wavinahc9000v3
    components: [wavin_ahc9000]

uart:
  id: uart_wavin
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 38400

wavin_ahc9000:
  id: wavin
  uart_id: uart_wavin
  update_interval: 5s
  # module: ustepper  # enable extended RS485 guard timings for uStepper hats
  # Optional: half-duplex RS485 direction control (DE/RE). Only include ONE of these if needed.
  # flow_control_pin: GPIO23  # Preferred unified DE/RE control (driven HIGH only while transmitting)
  # tx_enable_pin: GPIO23     # Legacy always-on style driver enable (kept for compatibility)
  channel_01_friendly_name: "Bedroom"
  channel_02_friendly_name: "Living Room"
  channel_03_friendly_name: "Kitchen"

climate:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Living Room & Kitchen"
    members: [2,3]
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
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom Temperature"
    channel: 1
    type: temperature
```

### Example ESPHome YAML (explicit platforms)
This variant shows a single channel, a grouped climate, plus both battery and temperature sensors explicitly defined.

```yaml
external_components:
  - source: github://heinekmadsen/esphome_wavinahc9000v3
    components: [wavin_ahc9000]

uart:
  id: uart_wavin
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 38400

wavin_ahc9000:
  id: wavin
  uart_id: uart_wavin
  update_interval: 5s
  # flow_control_pin: GPIO23 # (optional) direction control
  # module: ustepper  # optional: slower RS485 turnaround for uStepper hardware
  channel_01_friendly_name: "Bedroom"
  channel_02_friendly_name: "Living Room"
  channel_03_friendly_name: "Kitchen"

climate:
  # Group climate (channels 2 & 3). Generated YAML would comment out single 2/3 climates.
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Living Room & Kitchen"
    members: [2,3]
  # Single channel climate (channel 1)
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom"
    channel: 1

sensor:
  # Battery sensor for channel 1
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom Battery"
    channel: 1
    type: battery
  # Ambient temperature sensor for channel 1
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bedroom Temperature"
    channel: 1
    type: temperature

If you're using the uStepper RS485 expansion (commonly paired with ESP32-C3 boards), add `module: ustepper` under the `wavin_ahc9000:` block. This enables the longer guard times and RX flushing from the timing-optimized branch to prevent truncated acknowledgements.
```

### Comfort Climate Example
If a channel has a detected floor probe you can expose an alternative "comfort" climate that reports the floor temperature as the current temperature and also surfaces the floor min/max limits as adjustable low/high targets.

Add `use_floor_temperature: true` to a single-channel climate. You can optionally keep both the normal (air based) and comfort variant for the same physical zone (give them different names) – they will share the underlying setpoint.

```yaml
climate:
  # Standard air temperature climate
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bathroom"
    channel: 4

  # Comfort (floor-based) variant – requires a valid floor probe reading
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    name: "Bathroom Comfort"
    channel: 4
    use_floor_temperature: true
```

Notes:
* The comfort climate only appears in generated YAML if the floor probe was already detected (plausible >1°C and <90°C reading). If you add it manually and the probe is not yet detected it will show as unavailable until a valid value is read.
* Low / high target temperatures correspond to the controller's floor min / max limits. Adjusting them writes those limits (clamped 5–35°C, 0.5°C steps with at least 1.0°C separation enforced).
* Action (heating/idle) logic still derives from the difference between current (floor) temperature and the setpoint with a small hysteresis.

## Hardware & Wiring
* RS‑485 (A/B) from controller to a TTL↔RS‑485 adapter.
* ESP32 recommended (tested pins 16/17 for stable UART).
* Optional direction control:
  * `flow_control_pin:` supply a single GPIO tied to DE & /RE (HIGH during TX, LOW for RX). Recommended for most MAX3485/75176 style boards.
  * `tx_enable_pin:` legacy boolean driver enable (HIGH enables, left HIGH between frames). Use only if you already wired it this way; otherwise prefer `flow_control_pin`.
  * Avoid ESP32 strapping pins for DE/RE (GPIO0, GPIO2, GPIO4, GPIO5, GPIO12, GPIO15). Good choices include GPIO18/19/21/22/23/25/26/27/32/33.
* If neither is specified and your transceiver auto‑enables, you can omit both.

### Choosing flow_control_pin vs tx_enable_pin
| Option | Behavior | Pros | Cons |
|--------|----------|------|------|
| `flow_control_pin` | Pulsed HIGH only while sending, LOW for receive | Minimizes bus contention; cleaner half‑duplex | Very slightly more GPIO toggling |
| `tx_enable_pin` | HIGH enables driver (often kept HIGH) | Simple if already wired | Can hold bus driver enabled longer than needed |

Note on pins: Avoid strapping pins (GPIO0, 2, 4, 5, 12, 15) to prevent boot mode issues. Prefer GPIO23 in examples.

Prefer `flow_control_pin` for new builds. Keep `tx_enable_pin` only for backward compatibility or where hardware expects a static enable.

## Two-Phase Workflow (Recommended)

1. Generation phase (minimal config + generator package) → see `example_generate_yaml.yaml`.
2. Final phase (copied entities, generator removed) → see `example_after_generate_yaml.yaml`.

This keeps the final runtime lean while letting the component propose accurate entities.

See `INSTALLATION.md` for a detailed, step-by-step onboarding guide (generation → stitching → final deploy) plus troubleshooting tips.

Note: climates are defined explicitly under the `climate:` section using platform `wavin_ahc9000` (single channel or grouped). Optional per-channel battery sensors use `sensor: - platform: wavin_ahc9000`.

## Friendly Names
Provide any subset of:
```yaml
wavin_ahc9000:
  channel_01_friendly_name: "Bedroom"
  channel_07_friendly_name: "Office"
```
Missing entries fallback to `Zone N`.

Group climate naming (all members have friendly names):
* 2 members: `NameA & NameB`
* 3–4 members: `NameA, NameB & NameC`
* >4 members: `FirstName – LastName`
Fallback: `Zone G a&b` or `Zone G first-last`.

Single climates that belong to a generated group are still included in the full suggestion but commented out for clarity.

## Aggregation Semantics
| Property | Group Logic |
|----------|-------------|
| Current Temperature | Average of members (or floor temp for comfort variant) |
| Setpoint | Average of member setpoints |
| Action | Heating if any member heating else idle |

## Developer / Debug Tips
* Start with one known-good channel powered & paired.
* Use `logger:` level DEBUG while validating wiring.
* Temporarily raise `poll_channels_per_cycle` to accelerate discovery, then revert.
* Floor probe detection waits for plausible readings; early YAML generation may omit comfort climates—re-run later.

## Floor & Comfort Climates
Comfort climates appear only for channels with a detected floor probe. Names append `Comfort` to the friendly name (or `Zone N`).

## Child Lock Switches
Each thermostat channel exposes an inferred child lock bit (observed change 0x4000 → 0x4800 in the packed configuration register; mask `0x0800`). This component surfaces that as an optional per‑channel switch.

### When to Use
Lock the physical thermostat interface (prevent local user temperature changes) while still allowing Home Assistant / ESPHome to adjust setpoints programmatically. Turning the switch ON sets the lock; OFF clears it.

### YAML Example
```yaml
switch:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    channel: 9
    # type: child_lock   # optional (defaults to child_lock for now)
    name: "Office Lock"
```

Multiple channels:
```yaml
switch:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    channel: 3
    name: "Hall Lock"
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    channel: 7
    name: "Guest Lock"
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    channel: 9
    name: "Office Lock"
```

### Behavior & Notes
* Writes use a masked register update preserving unrelated bits.
* The switch publishes its optimistic state immediately; an urgent refresh confirms or corrects it within the next polling cycle.
* Safe to add/remove at any time (no reboot needed beyond normal ESPHome deployment cycle).
* If you rarely toggle locks you can omit them from the final config to keep the entity list lean – re‑add later if needed.

### Automation Samples
Lock all relevant zones at night:
```yaml
automation:
  - alias: Lock Wavin Zones At Night
    trigger:
      - platform: time
        at: "22:30:00"
    action:
      - service: switch.turn_on
        target:
          entity_id:
            - switch.office_lock
            - switch.hall_lock
            - switch.guest_lock
```

Unlock in the morning:
```yaml
automation:
  - alias: Unlock Wavin Zones Morning
    trigger:
      - platform: time
        at: "06:30:00"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.office_lock
```

### Troubleshooting
| Symptom | Suggestion |
|---------|------------|
| Switch state flips back | Underlying write failed (bus issue); check logs at DEBUG for masked write result |
| Switch always off | Channel not yet fully discovered; wait until first packed read (discovery phase) |
| No switch entity | YAML omitted switch platform or validation failed – ensure indentation & `platform: wavin_ahc9000` |

### Safety Considerations
Child lock blocks manual thermostat adjustments. Ensure you have reliable automation or a fallback (e.g., override climate setpoint) before mass‑locking all zones.

## Commented Single Climates for Group Members
When a group climate is generated, its member single-channel climates remain in the full suggestion but are commented out with an explanatory line. This keeps copy/paste flexible (just uncomment if you later decide to manage them individually).

## Generating YAML Suggestions (Log Output)

The component still discovers active channels and can draft entity definitions for you. Instead of exposing them through Home Assistant text sensors, the suggestion is now printed directly to the ESPHome log when you call `generate_yaml_suggestion()`.

Add a small API helper (or call from the ESPHome console) to trigger the dump:

```yaml
logger:
  level: INFO  # or DEBUG if you want verbose polling details

api:
  services:
    - service: dump_wavin_yaml
      then:
        - lambda: 'id(wavin).generate_yaml_suggestion();'
```

After the initial discovery phase (typically 30–60 seconds), call `esphome.<node_name>_dump_wavin_yaml` from Home Assistant or the ESPHome dashboard. The log output is wrapped between:

```
==================== Wavin YAML SUGGESTION BEGIN ====================
... entity blocks ...
===================== Wavin YAML SUGGESTION END =====================
```

Copy the blocks you need (climates, sensors, child lock switches) into your node YAML. Friendly names supplied via `channel_XX_friendly_name` are used to build readable entity names, and single-channel climates belonging to a group remain commented for easy opt-in. Re-run the service any time you rename channels or wire new zones; the output always reflects the latest discovery state.

Tip: if a floor probe or new thermostat has not reported yet, run the service again after a few minutes so the suggestion can include comfort climates and associated sensors.

### Comfort Setpoint (Read-Only) Sensor

If you do not want writable number entities but still want to surface the current comfort (manual) setpoint as a sensor for dashboards or historical graphs, you can add a `comfort_setpoint` sensor type:

```yaml
sensor:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    channel: 3
    type: comfort_setpoint
    name: "Zone 3 Comfort Setpoint"
```

This sensor publishes the latest manual/comfort setpoint (register 0x00) that the integration already reads for the climate entity. It is read-only; changing it requires using the climate entity or a number entity if enabled in another branch.

### Floor Temperature Sensor

If your thermostat provides a floor probe value, you can expose it separately:

```yaml
sensor:
  - platform: wavin_ahc9000
    wavin_ahc9000_id: wavin
    channel: 3
    type: floor_temperature
    name: "Zone 3 Floor Temperature"
```

This reads element index 0x05 (same scaling as air). A plausibility filter (>1..90°C) is applied; invalid or missing values result in the sensor staying unavailable.

Automatic detection: The component now marks a channel as having a floor sensor only after at least one plausible, non-zero floor reading is observed (> 1.0°C and < 90°C). Until then, any configured `floor_temperature` sensor will remain unavailable (rather than showing 0.0). The YAML generation services (`_wavin_generate_yaml` / `_wavin_publish_yaml_text_sensors`) will include floor temperature sensor entity suggestions only for channels where a valid floor probe has already been detected. If you generate YAML immediately after boot and a floor probe wasn’t yet detected, just trigger the service again later after some polling cycles.

## Future Ideas
* Optional toggle to suppress (instead of comment) grouped member single climates in generated YAML.
* Additional diagnostics (timing stats, packet counters) exposed via sensors.
* Entity category refinement for sensors (diagnostic vs primary).
* Bulk child lock service (lock/unlock multiple channels in one call).

## Disclaimer
Community-driven integration; use at your own risk. Not affiliated with Wavin / Jablotron.