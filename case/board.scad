// Mock-up of the ESP32 access control board, for fit checks only.
// Drawn in the board frame (see board_frame in common.scad).

module board_mock() {
    // PCB
    color("seagreen") difference() {
        cube([board_w, board_h, board_t]);
        for (p = board_holes)
            translate([bk(p)[0], bk(p)[1], -1]) cylinder(d = board_hole_d, h = board_t + 2);
    }
    // USB-C J4
    u = bk(usb_pos);
    color("silver") translate([u[0] - usb_depth / 2, u[1] - usb_body[0] / 2, board_t])
        cube([board_w - (u[0] - usb_depth / 2), usb_body[0], usb_body[1]]);
    // Wiegand terminals J5, J6. Wire entry faces v = 0 (the right wall).
    for (p = wago_pos) {
        c = bk(p);
        color("darkorange")
            translate([c[0] - wago_size[0] / 2, c[1] - wago_size[1] / 2, board_t])
                cube(wago_size);
    }
    // Pin headers J1 (aux) and J2 (display), both along the top edge of the PCB.
    color("dimgray") {
        translate([j1[0] - 1.27, board_h - j1[1] - 1.27, board_t])
            cube([j1[2] * 2.54, 2.54, board_comp_h]);
        translate([j2[0] + 1.27 - j2[2] * 2.54, board_h - j2[1] - 1.27, board_t])
            cube([j2[2] * 2.54, 2.54, board_comp_h]);
    }
    // Reset and boot buttons
    color("black") for (p = [[12.5, 48.75], [23.5, 48.75]]) {
        c = bk(p);
        translate([c[0] - 2.55, c[1] - 2.55, board_t]) cube([5.1, 5.1, 1.5]);
    }
}
