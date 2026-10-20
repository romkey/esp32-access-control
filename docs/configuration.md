# Configuration

There are no reusable packages yet. This documents the one config that is
built in CI. Packages and components get a section each when they are added.

## `examples/simple.yaml`

A bring-up config for the ESP32-S3 board: switch things on, read a card, see it
in Home Assistant or the web UI. Device name `led-switch`.

**Requires:** `secrets.yaml` with `wifi_ssid` and `wifi_password`. Copy
`examples/secrets.yaml.example` and edit it.

| Entity | Pin | Meaning |
|--------|-----|---------|
| `LED` switch | GPIO8 | The on-board LED |
| `Reader LED` switch | GPIO16 | Reader's LED line, active low |
| `Reader Beep` switch | GPIO15 | Reader's beeper line, active low |
| Wiegand D0 / D1 | GPIO18 / GPIO17 | Reader 1 data lines |

| Text sensor | Shows |
|-------------|-------|
| `Last Tag` | The tag as ESPHome decodes it |
| `Last Tag Raw` | Hex value of the last frame |
| `Last Tag H10301` | `facility,card` for a 26-bit HID H10301 frame that passes parity |

A good read runs the `acknowledge` script: a 150 ms beep and a 500 ms reader LED flash.

**Known gaps** (this is a bring-up config, not yet at the rules' standard): no
`esphome: project:`, no encrypted API, no fallback hotspot or safe mode, the
web server has no authentication, and the reader's second interface is unused.

```bash
docker compose run --rm validate
docker compose run --rm build
```
