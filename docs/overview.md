# esp32-access-control overview

**What:** an ESP32-S3 board with two level-shifted Wiegand reader interfaces,
its ESPHome firmware, and a 3D-printable kiosk case.
**Kind:** app (hardware project with firmware; no reusable packages yet).
**Hardware:** ESP32-S3-WROOM-1 N16R8 (16MB flash, octal PSRAM), Wiegand
readers on J5/J6, optional SPI TFT on J2, Qwiic I2C on J3, USB-C on J4.
No I2C devices on the board.

## Files

- `hw/reader/<version>/`: KiCad project, one directory per board revision.
- `BOFFF.md`: generated board outline, mounting holes, connector positions.
- `examples/simple.yaml`: bring-up config (LED switch, reader LED and beeper,
  Wiegand tag reading). All board and pin choices live in examples.
- `examples/secrets.yaml.example`: dummy secrets CI builds with.
- `case/`: OpenSCAD kiosk case. `make -C case` renders STLs; see `case/README.md`.
- `datasheets/`: manufacturer PDFs, each with a `-license.txt`.
- `scripts/`: tooling (list-configs, release, secrets and datasheet checks).
- `docs/configuration.md`: what the example config does and needs.

## Required by the example

- Secrets: `wifi_ssid`, `wifi_password`.
- Wiegand needs ESPHome 2025.3 or newer.

## Conventions

- Work on a branch; open a PR. CI and Lint must pass.
- Don't commit real secrets or firmware images (`.bin`); hooks reject both.
- Don't reformat KiCad files. `case/` is OpenSCAD and has its own README.
- Rules: https://github.com/romkey/esphome-ai-coding-rules/blob/main/RULES.md
