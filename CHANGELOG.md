# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- Kiosk case in `case/`: OpenSCAD, 3D-printable, for the board with a 3.5" or
  4.0" SPI TFT and an HID ProxPoint Plus reader, in portrait and landscape
  layouts, with an optional desk stand. The Case workflow renders the STLs.
- Project tooling from esphome-ai-coding-rules v0.6.1: Docker Compose services
  for validate, build, firmware export, lint and secret scanning; pre-commit
  hooks; CI, Lint and Release workflows; Renovate configuration; helper
  scripts; AGENTS.md; SECURITY.md; docs/overview.md and docs/configuration.md.
- License note for each datasheet, and the MIT `LICENSE`.

- Door-controller firmware, brought to the rules' layout: `components/`
  (`access_db`, `access_site`, `access_syslog`, `push_auth`), `packages/`
  (`access-door`, `push-auth`, `access-db`, `access-site`, `door-relays`,
  `wiegand-reader`), `examples/access-control.yaml` and
  `examples/common/base.yaml`. Pins are substitutions; the door relay pins are
  placeholders (GPIO38/39) until the wiring is decided.

### Changed

- `examples/simple.yaml` now includes `examples/common/base.yaml`: encrypted
  API and OTA, fallback hotspot, safe mode and factory reset buttons, and a
  web server with authentication.
- Secrets that were hard-coded in the firmware config (API key, OTA password,
  fallback hotspot password) are now `!secret`; new secrets `push_auth_token`
  and `syslog_server`. The old values were never committed.

- `docker-compose.yaml` replaced: the old one was left over from another
  project (it mounted a `src/` directory that doesn't exist here).
- `firmware/simple.yaml` moved to `examples/simple.yaml`, where CI builds
  device configs with the dummy secrets.
- README follows the rules' layout: badges, AI disclosure, project breakdown,
  technical details, build and flash instructions.
- `.gitignore` covers `secrets.yaml`, `local*.yaml`, firmware images and
  `.esphome/` anywhere in the tree.
