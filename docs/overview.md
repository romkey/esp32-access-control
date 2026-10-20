# esp32-access-control overview

**What:** an ESP32-S3 board with two level-shifted Wiegand reader interfaces,
its ESPHome firmware, and a 3D-printable kiosk case.
**Kind:** app (hardware project with firmware). The door-controller packages in `packages/` are reusable.
**Hardware:** ESP32-S3-WROOM-1 N16R8 (16MB flash, octal PSRAM), Wiegand
readers on J5/J6, optional SPI TFT on J2, Qwiic I2C on J3, USB-C on J4.
No I2C devices on the board.

## Files

- `hw/reader/<version>/`: KiCad project, one directory per board revision.
- `BOFFF.md`: generated board outline, mounting holes, connector positions.
- `examples/access-control.yaml`: the door controller (the real firmware).
- `examples/common/base.yaml`: shared project, API/OTA, recovery and web server settings.
- `packages/`: `access-door.yaml` (bundle), `push-auth`, `access-db`, `access-site`,
  `door-relays`, `wiegand-reader`. Board-agnostic; pins are substitutions.
- `components/`: C++ `access_db`, `access_site`, `access_syslog`, `push_auth`.
- `examples/simple.yaml`: bring-up config (LED switch, reader LED and beeper,
  Wiegand tag reading). All board and pin choices live in examples.
- `examples/secrets.yaml.example`: dummy secrets CI builds with.
- `case/`: OpenSCAD kiosk case. `make -C case` renders STLs; see `case/README.md`.
- `datasheets/`: manufacturer PDFs, each with a `-license.txt`.
- `scripts/`: tooling (list-configs, release, secrets and datasheet checks).
- `docs/configuration.md`: what the example config does and needs.

## Required by the example

- Secrets: `wifi_ssid`, `wifi_password`, `api_encryption_key`,
  `fallback_ap_password`, `web_server_username`, `web_server_password`, and for
  access-control.yaml `push_auth_token` and `syslog_server`.
- Wiegand needs ESPHome 2025.3 or newer.

## Notes

- `door-relays` is an example package for a relay or strike on the J1 aux header or a satellite board; `door_relay_1_pin` and `door_relay_2_pin` are required substitutions, and the example's GPIO38/39 are example values.
- Only reader 1 is wired in the examples; reader 2's pins aren't in BOFFF.md.
- Only an ESP32-S3 example exists, since the hardware is S3-only.

## Conventions

- Work on a branch; open a PR. CI and Lint must pass.
- Don't commit real secrets or firmware images (`.bin`); hooks reject both.
- Don't reformat KiCad files. `case/` is OpenSCAD and has its own README.
- Rules: https://github.com/romkey/esphome-ai-coding-rules/blob/main/RULES.md
