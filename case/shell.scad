// Rear part of the case. Carries the board, the cable openings and the
// wall-mount keyholes. Drawn in the case frame, Z = 0 at the back.

module shell() {
    difference() {
        union() {
            // Back plate
            linear_extrude(floor_t) outline();
            // Walls up to the bezel
            linear_extrude(Zi) difference() { outline(); outline(-wall); }
            // Tongue: the outer half of the wall continues into the bezel's rebate
            translate([0, 0, Zi - 0.01])
                linear_extrude(tongue_h + 0.01)
                    difference() { outline(); outline(-wall / 2); }

            // Board standoffs, on the floor
            board_frame(floor_t - 0.01)
                for (p = board_holes)
                    translate([bk(p)[0], bk(p)[1], 0])
                        cylinder(d = standoff_d, h = standoff_h + 0.01);
        }

        // Pilot holes for the M2 board screws
        board_frame(floor_t)
            for (p = board_holes)
                translate([bk(p)[0], bk(p)[1], standoff_h - 4.5])
                    cylinder(d = m2_pilot, h = 5);

        if (landscape) {
            // USB-C opening in the left wall, centred on the receptacle
            stadium_x(usb_slot[0], usb_slot[1], disp_cy, zb1 + usb_center_h,
                      -outer_w / 2 - 1, wall + 1.5);
            // Wiegand cable openings in the top wall, one per terminal block
            for (p = wago_pos)
                stadium_y(wire_slot_w, wire_slot_h,
                          board_w / 2 - bk(p)[0], zb1 + wire_slot_z,
                          inner_top - 0.5, wall + 1);
        } else {
            // USB-C opening in the top wall, centred on the receptacle
            stadium_y(usb_slot[0], usb_slot[1], 0, zb1 + usb_center_h,
                      inner_top - 0.5, wall + 1);
            // Wiegand cable openings in the right wall, one per terminal block
            for (p = wago_pos)
                stadium_x(wire_slot_w, wire_slot_h,
                          board_bot_y + bk(p)[0], zb1 + wire_slot_z,
                          inner_w / 2 - 0.5, wall + 1);
        }

        // Countersunk clearance holes for the M3 screws that go into the pillars
        for (p = pillar_pos)
            translate([p[0], p[1], -0.01]) {
                cylinder(d = m3_clear, h = floor_t + 0.02);
                cylinder(d1 = 6.4, d2 = m3_clear, h = 1.6);
            }

        // Keyholes for hanging the case on two screws
        for (k = kh_pos)
            translate([k[0], k[1], -0.01]) {
                cylinder(d = kh_big, h = floor_t + 0.02);
                hull() {
                    cylinder(d = kh_small, h = floor_t + 0.02);
                    translate([0, kh_slot, 0]) cylinder(d = kh_small, h = floor_t + 0.02);
                }
            }
    }
}

// Printed open side up, no supports needed.
module shell_print() { shell(); }
