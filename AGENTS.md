# Instructions for AI assistants

Follow https://github.com/romkey/esphome-ai-coding-rules/blob/main/RULES.md.
Follows esphome-ai-coding-rules v0.6.1. When re-scanning, apply the
entries in its UPGRADING.md newer than that, then update this line.
With Claude Code, the `esphome-rules` plugin loads them for you.

Read docs/overview.md first. `main` is protected: work on a branch and
open a pull request; CI and Lint must pass before it can merge.

<!-- Project-specific notes go below. -->

## This project

This is an **application** (a device people build and flash), not a package
library, and it is mostly hardware: the ESPHome firmware is one part of three.

- `hw/reader/<version>/` is KiCad. Don't reformat or "tidy" KiCad files, and
  keep pre-commit's whitespace hooks away from them (they are excluded).
  `BOFFF.md` is generated from the KiCad files with `kilint bofff . > BOFFF.md`;
  regenerate it rather than editing it.
- `case/` is OpenSCAD, not ESPHome. Build it with `make -C case`; the
  **Case** workflow does the same. Open `case/case.scad`, never the other
  `.scad` files. Dimensions of the board, display and reader are in
  `case/config.scad` with their sources.
- The MCU is an ESP32-S3-N16R8: 16MB flash, octal PSRAM. GPIO35-37 are used
  by the PSRAM; never assign them.
- The board has two Wiegand readers. Wiegand lines are level-shifted to 5V,
  so GPIOs are 3.3V; the pin assignments are in `hw/` and BOFFF.md, not
  something to guess.

### Not yet at the rules' layout

Device configs belong in `examples/` with reusable logic in `packages/` and
C++ in `components/` (RULES.md section 1). Only `examples/simple.yaml`, a
bring-up config, is there so far. The real firmware (`firmware/*.yaml`,
`firmware/components/`) is still local, uncommitted work in progress; some
of it hard-codes keys and passwords that must move to `secrets.yaml` before
it is committed (the `esphome-secrets` hook will refuse it). Don't commit it
as it is, and don't move it without being asked. `scripts/list-configs.sh`
builds only `examples/` until firmware publishing is set up.
