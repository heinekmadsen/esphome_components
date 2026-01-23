# Components in this repository

ESPHome is deprecating the local `custom_components/` folder (it will be removed in 2026.6.0).
To use components from this repository going forward, reference them via `external_components`.

## Recommended: external_components (ESPHome 2026.x)

Add this to your device YAML:

```yaml
external_components:
	- source:
			type: git
			url: https://github.com/heinekmadsen/esphome_components
			ref: main
		refresh: 0s
```

Then use the components as normal, e.g. for Wavin AHC9000 v2 you can keep using the packaged YAML files:

```yaml
packages:
	wavinahc9000v2:
		url: https://github.com/heinekmadsen/esphome_components
		ref: main
		files:
			- components/wavinahc9000v2/configs/basic.yaml
			- components/wavinahc9000v2/configs/channel_01.yaml
		refresh: 0s
```

## Migrating from custom_components

- Remove (or stop using) the local `custom_components/` copy of this repo.
- Add `external_components:` as shown above.
- Keep your existing YAML config; only the component source location changes.
