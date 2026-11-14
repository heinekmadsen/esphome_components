# Wavin AHC9000 Zones — Home Assistant integration

This custom integration turns the dynamic zone topology published by the
`wavinahc9000v4` ESPHome component into Home Assistant devices and entities at
runtime.

- One Home Assistant device per AHC zone, named `Zone (1+2)` initially.
- Dynamic climate, temperature, floor-temperature, battery, and child-lock
  entities.
- Zones added, removed, or re-grouped are reconciled without reflashing.
- The ESPHome native API is used directly; no MQTT broker is required.

## Requirements

- ESPHome firmware using `wavinahc9000v4` with `auto_zones: true`.
- A `text_sensor` of `type: topology`.
- The `wavin_zone_set_target`, `wavin_zone_set_mode`,
  `wavin_zone_set_child_lock`, and `wavin_zone_discover` API services.
- The ESPHome device reachable from Home Assistant on port `6053`.

See `components/wavinahc9000v4/example_auto.yaml` for a complete firmware
configuration.

## Installation

Copy `custom_components/wavin_ahc9000` into the Home Assistant `custom_components`
directory, restart Home Assistant, then add the **Wavin AHC9000 Zones**
integration and enter the ESPHome host/IP, port, and Noise PSK if used.

This is a feature-branch prototype and is intentionally not published yet.
