# Wavin AHC9000 — ESPHome V4

This is the ESPHome firmware for a Wavin AHC9000 / Jablotron AC-116 heating
controller. It automatically finds the heating zones and works with the
companion Home Assistant integration:

**[Wavin Floor Heating integration](https://github.com/nic6911/ha-wavin-floor-heating)**

[![Open in Marketplace](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=nic6911&repository=ha-wavin-floor-heating&category=integration)

<a href="https://github.com/nic6911/ha-wavin-floor-heating"><img src="docs/integration-repo-qr.png" width="170" alt="QR code for the Home Assistant integration"></a>

No MQTT. One Home Assistant device per zone. No per-channel YAML editing.

---

## Start here

### 1. Wire the ESP32

Connect the AHC RS-485 A/B wires to your ESP32 RS-485 module.

For the ModBridge Rev2 hardware, use the configuration in
**[example_auto.yaml](example_auto.yaml)**:

- UART TX: GPIO21
- UART RX: GPIO20
- RS-485 direction: GPIO3
- Baud rate: `38400`

For another board, change the UART pins in the YAML.

### 2. Flash the firmware

1. Open ESPHome.
2. Create a new device.
3. Paste **[example_auto.yaml](example_auto.yaml)**.
4. Set your Wi-Fi credentials.
5. Install it to the ESP32.

The firmware starts scanning all 16 AHC channels and publishes the zones it
finds. Leave it running.

### 3. Install the Home Assistant integration

1. Go to **Settings → Marketplace**.
2. Select **Add from link**.
3. Enter this repository:

   `https://github.com/nic6911/ha-wavin-floor-heating`

4. If Marketplace asks for a type, choose **Integration**.
5. Open **Wavin Floor Heating** and select **Install**.
6. If Home Assistant asks to restart, restart it.
7. Home Assistant normally discovers the module. Select **Configure** on the
   discovered **Wavin AHC9000** card and confirm. No IP address is needed.
8. If discovery does not appear, go to **Settings → Devices & services →
   Add integration**, select **Wavin AHC9000 Zones**, and enter the ESP32
   address as a fallback. Leave the Noise key empty unless you enabled ESPHome
   API encryption.

The zones appear as devices named like `Zone (1+2)` or `Zone (16)`.

---

## What you get

- One device per AHC zone
- Thermostat with heat/off and setpoint control
- Current temperature and battery
- Floor temperature and floor limits only on IR thermostats
- Child lock switch
- ESP32 Wi-Fi RSSI
- Optional **Sync all zones to zone (x)** switch

Setpoints follow the physical wall thermostat. Home Assistant only writes a
setpoint when you change it there.

## Automatic zone changes

When you add, remove, or regroup channels in the AHC controller, the firmware
picks it up automatically. No recompile and no YAML editing.

The integration adds or removes the Home Assistant devices on the next update.

## Floor sensors

The AHC9000 reports floor-sensor presence per channel. The integration creates
floor entities only where an IR thermostat actually has a floor probe:

- `Floor temperature`
- `Floor limits` climate with low/high floor temperature settings
- `Floor control` switch for the controller's floor-sensing enable bit

Thermostats without a floor probe get none of these entities.

## Sync all zones

The sync switch copies the first available zone's setpoint to all other zones.
Turn it off to make the zones independent again.

If you rename the master zone device in Home Assistant, the sync switch text
follows that name.

## Troubleshooting

| Problem | Try |
|---|---|
| No zones in HA | Check that the firmware uses `auto_zones: true` and has the `topology` text sensor |
| Cannot add the integration | Check the ESP32 IP and that TCP port `6053` is reachable |
| Zones show but controls fail | Update firmware and integration together |
| ESP32 offline | Check Wi-Fi and the RSSI sensor |
| Old zone names or devices remain | Reload the integration |

## Advanced

Normal installations should use the automatic mode above.

For manual per-channel YAML, external room sensors, floor-probe climates, child
lock, repair buttons, and all component options, see
**[MANUAL.md](MANUAL.md)**.

The only hardware-specific example files are:

- **[example_auto.yaml](example_auto.yaml)** — normal automatic setup
- **[example_manual.yaml](example_manual.yaml)** — advanced manual setup

## Firmware family

- `wavinahc9000v4` — automatic zones + Home Assistant integration
- Legacy `wavinahc9000v3` — previous manual/community component

Use V4 for new installations.
