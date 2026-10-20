#!/usr/bin/env python3
"""Copy compiled firmware images out of ESPHome's build tree.

    scripts/export-firmware.py OUTDIR CONFIG...

Run after `esphome compile` (docker-compose.yaml's `firmware` service does both).
For each config, copies <name>.factory.bin (the full image for a blank
chip, e.g. a first flash with https://web.esphome.io) and <name>.ota.bin to
OUTDIR. Builds live in a Docker volume, so this is how they reach the host.

These images contain the Wi-Fi credentials they were built with: keep them
out of git (dist/ is ignored) and don't share them.
"""

import json
import shutil
import sys
from pathlib import Path


def find(build: Path, filename: str) -> Path | None:
    # Native ESP-IDF builds and PlatformIO builds put images in different
    # places under the build path.
    return next(iter(sorted(build.rglob(filename))), None)


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__, file=sys.stderr)
        return 2
    out = Path(argv[0])
    out.mkdir(parents=True, exist_ok=True)
    status = 0
    for config in map(Path, argv[1:]):
        storage = config.parent / ".esphome" / "storage" / f"{config.name}.json"
        if not storage.is_file():
            print(f"{config}: no build info; compile it first", file=sys.stderr)
            status = 1
            continue
        info = json.loads(storage.read_text())
        name, build = info["name"], Path(info["build_path"])
        for kind in ("factory", "ota"):
            image = find(build, f"firmware.{kind}.bin")
            if image is None:
                print(
                    f"{config}: no firmware.{kind}.bin under {build}", file=sys.stderr
                )
                status = 1
                continue
            target = out / f"{name}.{kind}.bin"
            shutil.copyfile(image, target)
            print(f"{config} -> {target}")
    return status


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
