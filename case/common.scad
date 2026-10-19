// Shared helpers. Included by case.scad after config.scad.

// 2D rounded rectangle, centred on X, sitting on Y = 0.
module rrect(w, h, r) {
    translate([-w / 2, 0]) offset(r = r) offset(delta = -r) square([w, h]);
}

// Outline of the case. Positive `grow` makes it bigger.
module outline(grow = 0) {
    offset(delta = grow) rrect(outer_w, outer_h, corner_r);
}

// Convert KiCad board coordinates (origin top-left, Y down) to board
// frame coordinates (u along the 95 mm length, v up the 55 mm width,
// with v = 0 on the WAGO edge).
function bk(p) = [p[0], board_h - p[1]];

// Place children in the board's own frame. Z is measured from `z`.
// Portrait: the board is turned 90 degrees, its USB-C edge faces the top
//           of the case and its WAGO edge faces the right side wall.
// Landscape: the board is turned 180 degrees, its USB-C edge faces the left
//           side wall and its WAGO edge faces the top wall.
module board_frame(z = 0) {
    if (landscape)
        translate([0, disp_cy, z])
            rotate([0, 0, 180])
                translate([-board_w / 2, -board_h / 2, 0])
                    children();
    else
        translate([0, board_bot_y, z])
            rotate([0, 0, 90])
                translate([0, -board_h / 2, 0])
                    children();
}

// Place children in the display's frame: centred, +Y toward the header.
// Z = 0 is the inside face of the bezel plate; the display extends to -Z.
// Landscape turns the display a quarter turn so the header end is on the left.
module display_frame() {
    translate([0, disp_cy, Zi])
        rotate([0, 0, (landscape ? 90 : 0) + (display_flip ? 180 : 0)])
            children();
}

// A stadium (slot with round ends) cut along +Y, centred at x, z.
module stadium_y(w, h, x, z, y0, len) {
    hull() for (s = [-1, 1])
        translate([x + s * (w - h) / 2, y0, z])
            rotate([-90, 0, 0]) cylinder(d = h, h = len);
}

// A stadium cut along +X, centred at y, z.
module stadium_x(w, h, y, z, x0, len) {
    hull() for (s = [-1, 1])
        translate([x0, y + s * (w - h) / 2, z])
            rotate([0, 90, 0]) cylinder(d = h, h = len);
}
