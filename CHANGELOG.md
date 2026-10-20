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

### Changed

- `docker-compose.yaml` replaced: the old one was left over from another
  project (it mounted a `src/` directory that doesn't exist here).
- `firmware/simple.yaml` moved to `examples/simple.yaml`, where CI builds
  device configs with the dummy secrets.
- README follows the rules' layout: badges, AI disclosure, project breakdown,
  technical details, build and flash instructions.
- `.gitignore` covers `secrets.yaml`, `local*.yaml`, firmware images and
  `.esphome/` anywhere in the tree.
