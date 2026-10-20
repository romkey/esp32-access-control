# esp32-access-control

[![CI](https://github.com/romkey/esp32-access-control/actions/workflows/ci.yaml/badge.svg)](https://github.com/romkey/esp32-access-control/actions/workflows/ci.yaml)
[![Lint](https://github.com/romkey/esp32-access-control/actions/workflows/lint.yaml/badge.svg)](https://github.com/romkey/esp32-access-control/actions/workflows/lint.yaml)
[![Case](https://github.com/romkey/esp32-access-control/actions/workflows/case.yml/badge.svg)](https://github.com/romkey/esp32-access-control/actions/workflows/case.yml)
[![ESPHome](https://img.shields.io/badge/ESPHome-firmware-18BCF2?logo=esphome&logoColor=white)](https://esphome.io/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

An open source, ESP32-based Wiegand reader: a board, ESPHome firmware and a 3D-printable case.

At PDX Hackerspace we used RFID keys to unlock doors and access our laser cutter. We want to expand their use, to make it easy for members to print parking permits for projects, unlock the main entrance for events, claim prints on 3D printers and more. We currently use Raspberry Pis for the doors and an ESPKey for the laser cutter; this project can provide a low cost simple way to integrate with Wiegand RFID readers and possibly as the basis for replacing the aging Raspberry Pis controlling our doors.

> **AI disclosure:** This project was written by AI under human oversight,
> following [these rules](https://github.com/romkey/esphome-ai-coding-rules/blob/main/RULES.md).

## What's here

| Path | What it is |
|------|------------|
| `hw/reader/<version>/` | KiCad project for the board, one directory per revision |
| `BOFFF.md` | Bill of Form, Fit and Function: board outline, mounting holes and connector positions, for designing enclosures |
| `examples/access-control.yaml` | The door controller: Wiegand reader, access database, door relay, syslog |
| `examples/simple.yaml` | ESPHome config for bring-up: LED switch, reader LED and beeper, Wiegand tag reading |
| `examples/common/base.yaml` | Settings every example shares: encrypted API and OTA, recovery, web server |
| `packages/` | Board-agnostic ESPHome packages for the door controller; pins come from the including config |
| `components/` | C++ external components: `access_db`, `access_site`, `access_syslog`, `push_auth` |
| `case/` | OpenSCAD design for a wall or desk kiosk in portrait and landscape layouts; see [case/README.md](case/README.md) |
| `datasheets/` | Manufacturer datasheets, each with a license note |
| `docs/` | [Overview](docs/overview.md) and [configuration reference](docs/configuration.md) |

## Technical details

Hardware:

- dual level-shifted 5V Wiegand interfaces with wiring for data (D0, D1), LED and beep along with power
- ESP32-S3-N16R8: ESP32-S3 with 16MB of flash and 8MB of octal PSRAM (GPIO35-37 are used by the PSRAM and unavailable)
- USB C port
- power and data LEDs
- Qwiic connector for I2C devices
- header for SPI TFT displays with touch controllers
- "aux" header for satellite boards with optional power to this board

Connectors, from [BOFFF.md](BOFFF.md):

| Ref | Part | What it is |
|-----|------|------------|
| J1 | 10-pin header | Aux: GND, 3.3V, TX0, RX0, IO42, IO41, IO40, IO39, IO38, +5V in |
| J2 | 14-pin header | SPI TFT with touch: 3.3V, GND, CS, RST, DC, SDO, SCK, LED, SDI, then T_CLK, T_CS, T_DIN, T_DO, T_IRQ |
| J3 | SM04B-SRSS-TB | Qwiic: GND, 3.3V, SDA, SCL |
| J4 | USB-C | Power and programming |
| J5, J6 | WAGO 2601-1106 | Wiegand readers 1 and 2: +5V, GND, then four signals (D0, D1, LED, beep; see the schematic for the order) |

I2C: SDA and SCL on J3, with 4.7k pull-ups to 3.3V. The board carries no I2C devices of its own.

`examples/simple.yaml` and `examples/access-control.yaml` wire one reader on D0 = GPIO18, D1 = GPIO17, reader LED = GPIO16
(active low) and beeper = GPIO15 (active low). Check these against the schematic before
relying on them. The board has no relay output; `packages/door-relays.yaml` is an example for a relay or strike driven from
the J1 aux header or a satellite board. `examples/access-control.yaml` uses GPIO38 and GPIO39 (J1) as example
values; set `door_relay_1_pin` and `door_relay_2_pin` for your own hardware.

Using the packages from another project:

```yaml
external_components:
  - source: github://romkey/esp32-access-control@vX.Y.Z
packages:
  access-door: github://romkey/esp32-access-control/packages/access-door.yaml@vX.Y.Z
```

See [docs/configuration.md](docs/configuration.md) for the substitutions each package takes.

Datasheets: [TXS0108E level shifter](datasheets/txs0108e.pdf),
[WAGO 2601-1106 terminal block](datasheets/2946_WAGO26011106enUS.pdf).

## Building

Requires Docker. Nothing else needs installing.

```bash
cp examples/secrets.yaml.example examples/secrets.yaml   # then edit it (generate a real api key)
docker compose run --rm validate      # esphome config on every example
docker compose run --rm build         # compile every example
docker compose run --rm lint          # all pre-commit checks, like CI
```

The first build downloads the toolchains into a Docker volume; later builds reuse
them and only recompile what changed.

The case is built separately with OpenSCAD: `make -C case`. See [case/README.md](case/README.md).

## Flashing

To flash a new board, build the firmware images:

```bash
docker compose run --rm firmware
```

Then open https://web.esphome.io in Chrome or Edge, connect the board over USB and upload
`dist/<name>.factory.bin`. After that, update over the network (OTA). The images contain
your Wi-Fi credentials; don't share them.

## Case

[`case/`](case/) has an OpenSCAD design for a 3D-printable wall or desk kiosk, in portrait and landscape layouts. It holds the board, a 3.5" or 4.0" SPI TFT, and an HID ProxPoint Plus reader mounted on the front. CI renders the STLs on every change and attaches them to the workflow run. See [`case/README.md`](case/README.md) for parts, print settings and assumptions to check.

## License

[MIT](LICENSE), except the manufacturer datasheets in [datasheets/](datasheets/),
which belong to their manufacturers and are not covered by this license; see
each datasheet's `-license.txt`.
