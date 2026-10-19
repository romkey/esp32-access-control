// Front part of the case. Holds the display and carries the reader.
// Drawn in the case frame: the plate occupies Z = Zi .. Zi + front_t.

module bezel() {
    difference() {
        union() {
            // Front plate
            translate([0, 0, Zi]) linear_extrude(front_t) outline();

            // Display bosses: the display PCB sits on these and is screwed to them
            display_frame()
                for (sx = [-1, 1], sy = [-1, 1])
                    translate([sx * disp_hole_dx / 2, sy * disp_hole_dy / 2, -disp_boss_h])
                        cylinder(d = disp_boss_d, h = disp_boss_h + 0.01);

            // Reader screw bosses
            for (p = reader_screws())
                translate([p[0], p[1], Zi - reader_boss_h])
                    cylinder(d = reader_boss_d, h = reader_boss_h + 0.01);

            // Pillars that take the M3 screws coming through the back of the shell
            for (p = pillar_pos)
                translate([p[0], p[1], floor_t + pillar_gap])
                    cylinder(d = pillar_d, h = Zi - floor_t - pillar_gap + 0.01);
        }

        // Rebate on the back edge: receives the outer half of the shell wall
        translate([0, 0, Zi - 0.01])
            linear_extrude(tongue_h + fit + 0.01)
                difference() {
                    outline(1);
                    outline(-(wall / 2 + fit));
                }

        // Display window
        display_frame()
            translate([0, -disp_l / 2 + disp_aa_off + disp_aa[1] / 2, -1])
                linear_extrude(front_t + 2)
                    offset(r = 1) offset(delta = -1)
                        square([disp_aa[0] + 2 * win_margin, disp_aa[1] + 2 * win_margin], center = true);

        // Pilot holes in the display bosses (M3)
        display_frame()
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * disp_hole_dx / 2, sy * disp_hole_dy / 2, -disp_boss_h - 1])
                    cylinder(d = m3_pilot, h = disp_boss_h + 1 + 1.2);   // stops 2.4 mm short of the front face

        // Reader screw pilot holes: through the plate and well into the boss
        for (p = reader_screws())
            translate([p[0], p[1], Zi - reader_boss_h + 1])
                cylinder(d = reader_pilot, h = reader_boss_h + front_t);

        // Reader cable hole
        translate([landscape ? reader_cable_dy : 0, reader_cy + (landscape ? 0 : reader_cable_dy), Zi - 1])
            cylinder(d = reader_cable_d, h = front_t + 2);

        // Pilot holes in the pillars, open at the bottom end
        for (p = pillar_pos)
            translate([p[0], p[1], floor_t + pillar_gap - 1])
                cylinder(d = m3_pilot, h = 1 + 16);

        // Engraved outline showing where the reader goes
        translate([0, reader_cy, Zi + front_t - reader_groove])
            linear_extrude(reader_groove + 0.01)
                difference() {
                    square([rdr_x + 0.8 + 2 * 0.8, rdr_y + 0.8 + 2 * 0.8], center = true);
                    square([rdr_x + 0.8, rdr_y + 0.8], center = true);
                }
    }
}

// Printed with the front face on the bed, so there is nothing to support.
module bezel_print() {
    translate([0, outer_h, Zi + front_t]) rotate([180, 0, 0]) bezel();
}
