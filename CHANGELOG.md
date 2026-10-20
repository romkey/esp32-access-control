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
- `.github/actionlint.yaml` teaches actionlint the `ubuntu-26.04` runner label the Case
  workflow uses; the pinned actionlint release doesn't know it yet.

- Door-controller firmware, brought to the rules' layout: `components/`
  (`access_db`, `access_site`, `access_syslog`, `push_auth`), `packages/`
  (`access-door`, `push-auth`, `access-db`, `access-site`, `door-relays`,
  `wiegand-reader`), `examples/access-control.yaml` and
  `examples/common/base.yaml`. Pins are substitutions; the door relay pins are
  placeholders (GPIO38/39) until the wiring is decided.

### Fixed

- The reader LED and beeper no longer stay on after a granted tag; the new
  `wiegand_acknowledge` script switches them off again.
- Access database uploads and unlock requests failed on current ESPHome
  (2026.9), which reads the POST body before the handler runs; both now take
  it in `handleBody`. Clients must send a non-form `Content-Type`
  (`application/json`).
- The syslog server may now be a hostname as well as an IPv4 address; it is
  resolved once and cached, and a failed lookup is retried every 30 seconds.
  IPv6 addresses are rejected at validation instead of failing at run time.
- A failed rename no longer deletes the live access database; the new file
  replaces it in one step.

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
