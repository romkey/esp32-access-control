# Kiosk case

[![Case](https://github.com/romkey/esp32-access-control/actions/workflows/case.yml/badge.svg)](https://github.com/romkey/esp32-access-control/actions/workflows/case.yml)

A 3D-printable indoor kiosk for the ESP32 access control board. It is not
watertight and not tamper resistant.

- The board stands upright behind the display.
- A 3.5" (ILI9488) or 4.0" (ST7796S) SPI TFT sits in a window in the front plate.
- An HID ProxPoint Plus 6005 screws to the outside of the front, below the display.
- USB-C leaves through the top. The two Wiegand terminal blocks leave through the right side.
- Hang it on two screws, or print the optional stand to stand it on a desk.

## Building

Open `case.scad` in OpenSCAD, or from the command line:

```bash
make            # every STL, both display sizes, into stl/
make png        # preview images into png/
```

CI runs `make stl` with OpenSCAD from Ubuntu's package archive, fails on any OpenSCAD warning, and uploads the STLs as a build artifact. `make` finds OpenSCAD on `PATH` or in `/Applications/OpenSCAD.app`.

To render one part:

```bash
openscad -D 'part="shell"' -D 'display="4.0"' -o shell-4.0.stl case.scad
```

`part` is `assembly`, `shell`, `bezel` or `stand`. `display` is `"3.5"` or `"4.0"`.
For `assembly`, `exploded=60` pulls the parts apart and `with_stand=true` seats the case in the stand.
Every dimension lives in `config.scad`.

## Parts

| File | What it is |
|---|---|
| `case.scad` | Top level. Open this one. |
| `config.scad` | All dimensions and options. |
| `common.scad` | Shared helpers. |
| `shell.scad` | Rear half: board standoffs, cable openings, keyholes. |
| `bezel.scad` | Front half: display window and bosses, reader holes, screw pillars. |
| `stand.scad` | Optional tilted desk stand. |
| `board.scad`, `display.scad`, `reader.scad` | Mock-ups of the real parts, used only in `assembly`. |

Print one shell, one bezel and, if you want it, one stand, all for the same display size.

| Part | 3.5" | 4.0" |
|---|---|---|
| shell | 64 × 197 × 34 mm | 70 × 207 × 34 mm |
| bezel | 64 × 197 × 33 mm | 70 × 207 × 33 mm |
| stand | 76 × 118 × 36 mm | 82 × 118 × 36 mm |

The case is tall. It needs a bed of about 220 mm, or print it diagonally on a smaller one.

## Print settings

- PLA or PETG, 0.2 mm layers, 3 perimeters, 20% infill. No supports.
- The shell prints open side up. The bezel prints front face down, which is why the
  reader outline is engraved rather than raised.
- Walls are 2.4 mm, so 0.4 mm nozzle perimeters line up.

## Hardware

| Qty | For |
|---|---|
| 4 | M2 × 6 self-tapping screws, board to standoffs |
| 4 | M3 × 16 flat head self-tapping screws, back of shell into the bezel pillars |
| 4 | M3 × 8 self-tapping screws, display to bezel bosses |
| 2 | #4 × 1" screws, supplied with the reader |
| 2 | Wall screws with a head under 8 mm, for the keyholes |
| 1 | 14-way female-to-female jumper ribbon, board J2 to the display header |

## Assembly notes

1. Screw the reader to the front. The two holes are 68.3 mm apart. Pass its cable through the 16 mm hole.
2. Screw the display to the four bosses on the inside of the bezel. The glass sits 0.3 mm behind the window.
3. Screw the board to the shell standoffs. Plug the display into board header J2 with the ribbon.
4. Feed the Wiegand cables through the right-hand openings and the USB cable through the top.
5. Close the shell onto the bezel. The outer half of the shell wall fits into a rebate in the bezel. Drive the four M3 screws in from the back.

By default the display's header end is at the top, next to J2. Set `header_top = false` to turn the display around, and rotate the picture in firmware to match.

## Things to check against your hardware

These came from datasheets and drawings, not from measuring the parts.

- **Reader hole spacing.** 68.3 mm (2.690") comes from a third-party template for the 6005. Hold the reader up to the bezel before printing and change `reader_screw_dy` if it is different. The cable hole is assumed to be midway between the screws.
- **Wiegand wire entry.** The terminal blocks are assumed to take wires from the board edge side. If yours enter from the top, the openings still work but the strain relief will not.
- **USB-C position.** The receptacle is assumed to sit flush with the board edge, 2.5 mm from the top wall.
- **Display variants.** Dimensions come from the lcdwiki MSP3520 and MSP4021 outline drawings. Other 3.5" and 4.0" modules, and the capacitive touch versions, have different outlines.
- **Fit.** Print the shell first and check the board and the cable openings before printing the bezel.

Board and display mock-ups in `assembly` are for checking clearances. They are not accurate models.

## Out of scope

Weatherproofing, tamper protection, light pipes for the board LEDs, and access to the reset and boot buttons with the case closed.
