// =====================================================================
// ESP32 access control kiosk case.
//
// Open this file. Choose the parts and display size in config.scad, or on
// the command line:
//   openscad -D 'part="shell"' -D 'display="4.0"' -o shell-4.0.stl case.scad
//
// part = "assembly"  everything, with mock-ups of the board, display and reader
//        "shell"     rear half, oriented for printing
//        "bezel"     front half, oriented for printing
//        "stand"     optional desk stand, oriented for printing
// =====================================================================

include <config.scad>
include <common.scad>
include <board.scad>
include <display.scad>
include <reader.scad>
include <bezel.scad>
include <shell.scad>
include <stand.scad>

module assembly_parts() {
    color("lightsteelblue") shell();
    color("slategray") translate([0, 0, exploded]) bezel();
    translate([0, 0, exploded * 0.5]) board_frame(zb0) board_mock();
    translate([0, 0, exploded * 0.8]) display_frame() display_mock();
    translate([0, 0, exploded * 1.5]) reader_mock();
}

module assembly() {
    if (with_stand) stand_with_case();
    else assembly_parts();
}

if (part == "shell")      shell_print();
else if (part == "bezel") bezel_print();
else if (part == "stand") stand_print();
else                      assembly();
