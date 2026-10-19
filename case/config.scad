// =====================================================================
// config.scad - every dimension used by the case, in millimetres.
//
// Do not open this file directly. Open case.scad, which includes it.
// Any variable here can be overridden on the command line with
//   openscad -D 'display="4.0"' -o out.stl case.scad
//
// Coordinate frame for the assembled case ("case frame"):
//   X  left/right across the front, 0 = centre line
//   Y  up the front, 0 = bottom of the outer shell
//   Z  front to back, 0 = outside of the back of the shell,
//      +Z points out of the front of the case at the viewer
// =====================================================================

// ---- Selection ------------------------------------------------------
display  = "3.5";       // "3.5" (ILI9488, MSP3520) or "4.0" (ST7796S, MSP4020/4021)
part     = "assembly";  // "assembly" | "shell" | "bezel" | "stand"
exploded = 0;           // assembly only: pull parts apart by this many mm
with_stand = false;     // assembly only: show the case seated in the stand
header_top = true;      // true: display's 14-pin header end is at the top,
                        //       next to the board's display header J2.
                        // false: rotated 180 degrees.
$fn = 48;

// ---- Printed part parameters ----------------------------------------
wall        = 2.4;      // side walls (six 0.4 mm perimeters)
floor_t     = 2.4;      // back of the shell
front_t     = 3.6;      // front plate of the bezel
tongue_h    = 2.4;      // outer half of the shell wall that rises into the bezel
fit         = 0.25;     // clearance between mating printed surfaces
corner_r    = 5;        // outside corner radius
side_gap    = 1.5;      // space between display PCB and inner side wall

// ---- Fasteners ------------------------------------------------------
m2_pilot    = 1.8;      // self-tapping M2 into the board standoffs
m3_pilot    = 2.6;      // self-tapping M3 into the bezel pillars / display bosses
m3_clear    = 3.4;      // M3 clearance
reader_pilot = 2.0;     // HID specifies a 2 mm (5/64") pilot for the #4 screws

// ---- ESP32 access control board (hw/reader, BOFFF.md) ----------------
// KiCad board coordinates: origin top-left, X right, Y DOWN.
board_w  = 95;
board_h  = 55;
board_t  = 1.6;
board_holes   = [[2.5, 2.5], [92.5, 2.75], [92.5, 52.25], [2.5, 52.5]];
board_hole_d  = 2.2;
usb_pos       = [89.625, 27.5];  // USB-C J4, on the right edge
usb_body      = [8.94, 3.26];    // width, height of the receptacle
usb_depth     = 7.3;             // body length measured from its centre toward the edge
wago_pos      = [[36, 41.5], [61, 41.5]];  // J5, J6 (2601-1106)
wago_size     = [22.5, 16.58, 12.75];      // along edge, depth, height above PCB
j1            = [20.13, 2.75, 10];         // aux header: start x, y, pins (runs +x)
j2            = [81.69, 2.75, 14];         // display header: start x, y, pins (runs -x)
board_comp_h  = 8.5;                       // pin header height above PCB

// ---- How the board sits in the case ---------------------------------
// The board stands upright: its WAGO edge faces the right side wall,
// its USB-C edge faces the top wall.
standoff_h   = 5;       // underside clearance (through-hole pins, keyholes)
standoff_d   = 6;
top_clear    = 1.0;     // gap above the tallest board part
usb_slot     = [14, 7.5];   // opening in the top wall (width, height)
usb_center_h = 1.63;        // receptacle centre above the PCB top
wire_slot_h  = 9;           // opening height for the Wiegand cables
wire_slot_z  = 5;           // slot centre above the PCB top
wire_slot_w  = 24;          // opening width, one per terminal block

// ---- Displays -------------------------------------------------------
// From the lcdwiki outline drawings MSP3520_Size.pdf / MSP4021_Size.pdf.
// Display frame: centred, +Y is the end with the 14-pin header.
is4          = display == "4.0";
disp_w       = is4 ? 61.74  : 56.34;   // PCB
disp_l       = is4 ? 108.04 : 98.00;
disp_hole_dx = is4 ? 54.90  : 49.50;
disp_hole_dy = is4 ? 102.04 : 92.00;
disp_hole_d  = 3.2;
disp_bl      = is4 ? [60.88, 94.57] : [55.50, 84.96];  // glass outline
disp_bl_off  = is4 ? 6.75 : 6.54;      // glass edge from the end opposite the header
disp_aa      = is4 ? [55.68, 83.52] : [48.96, 73.44];  // lit area
disp_aa_off  = is4 ? 9.40 : 9.95;      // lit area from the end opposite the header
disp_stack   = is4 ? 4.05 : 4.20;      // glass + touch panel in front of the PCB
disp_pcb_t   = 1.6;
disp_smd     = 3.0;                    // tallest part on the back of the PCB
disp_total   = is4 ? 14.00 : 14.18;    // glass front to tips of the header pins
disp_hdr_len = 33.02;                  // 14 pins at 2.54
disp_hdr_in  = 2.0;                    // pin row centre from the PCB end
disp_gap     = 0.3;                    // glass sits this far behind the bezel
win_margin   = 0.5;                    // window is lit area plus this per side

// ---- HID ProxPoint Plus 6005 ----------------------------------------
reader_w      = 43.7;   // 1.720"
reader_h      = 79.6;   // 3.135"
reader_t      = 16.8;   // 0.660"
reader_screw_dy = 68.3; // 2.690" between the two mounting screws
reader_cable_d  = 16;   // 5/8" cable hole
reader_cable_dy = 0;    // cable hole offset from the midpoint between the screws
reader_boss_h   = 14;   // material behind the plate for the screws to bite into
reader_boss_d   = 8;
reader_margin   = 4;    // gap from reader body to the bottom inner wall
reader_groove   = 0.4;  // engraved locating outline on the front face

// ---- Layout (derived) ------------------------------------------------
disp_reader_gap = 10;   // between reader body and display PCB
disp_top_gap    = 1.0;  // between display PCB and the top inner wall

inner_w   = disp_w + 2 * side_gap;
outer_w   = inner_w + 2 * wall;

reader_cy  = wall + reader_margin + reader_h / 2;
reader_top = reader_cy + reader_h / 2;
disp_bot   = reader_top + disp_reader_gap;
disp_cy    = disp_bot + disp_l / 2;
disp_top   = disp_bot + disp_l;
inner_top  = disp_top + disp_top_gap;
outer_h    = inner_top + wall;

board_bot_y = disp_top - board_w;   // board is top-aligned with the display

// Depth: the board sits on standoffs, the display sits behind the bezel.
zb0       = floor_t + standoff_h;   // underside of the board
zb1       = zb0 + board_t;          // top of the board
z_wago    = zb1 + wago_size[2];     // top of the tallest board part
disp_behind = disp_gap + disp_stack + disp_pcb_t + disp_smd;
Zi        = ceil(z_wago + top_clear + disp_behind);  // inside face of the bezel plate
case_d    = Zi + front_t;           // total depth of the case, excluding the reader

// Display bosses on the bezel
disp_boss_d = 6;
disp_boss_h = disp_gap + disp_stack;

// Pillars that carry the M3 screws from the back of the shell into the bezel
pillar_d   = 7;
pillar_x   = inner_w / 2 - pillar_d / 2 - 0.3;
pillar_pos = [[-pillar_x, wall + 5],
              [ pillar_x, wall + 5],
              [-pillar_x, reader_top + disp_reader_gap / 2],
              [ pillar_x, reader_top + disp_reader_gap / 2]];
pillar_gap = 0.2;       // pillar end stops this far above the floor

// Wall-mount keyholes in the back of the shell
kh_big   = 9;
kh_small = 4.6;
kh_slot  = 9;
kh_y     = [wall + 14, inner_top - 14];

// ---- Stand ----------------------------------------------------------
stand_tilt   = 10;      // degrees the case leans back
stand_wall   = 6;
stand_front  = 62;      // base length in front of the case's back face (clears the reader)
stand_rear   = 50;      // base length behind it; keeps the leaning case from tipping over
stand_pocket = 28;      // how deep the case sits in the stand, measured along its height
