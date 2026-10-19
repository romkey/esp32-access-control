// Optional desk stand: a wedge with a pocket the bottom of the case drops
// into, leaning back by stand_tilt degrees. The pocket is open at the front
// so the reader (which sticks out of the front face) clears it.
//
// Stand frame: X right, Y toward the back (the viewer is at -Y), Z up.
// The back face of the case sits at Y = stand_front + fit.

stand_w   = outer_w + 2 * stand_wall;
stand_len = stand_front + stand_wall + stand_rear;
stand_floor = 4;                       // thickness of the base slab

// Map the case frame into the stand frame.
module in_stand() {
    translate([0, stand_front, stand_floor])
        rotate([-stand_tilt, 0, 0])
            rotate([90, 0, 0])
                children();
}

module stand() {
    pocket_w = outer_w + 2 * fit;
    pocket_d = case_d + 2 * fit;
    rise     = stand_pocket * cos(stand_tilt) + 8;   // height of the cheeks
    difference() {
        // Slab with a raised block around the pocket; the hull makes ramps
        // down to the slab at the front and back.
        hull() {
            translate([-stand_w / 2, 0, 0]) cube([stand_w, stand_len, stand_floor]);
            translate([-stand_w / 2, stand_front - pocket_d * cos(stand_tilt) - 2, 0])
                cube([stand_w, pocket_d * cos(stand_tilt) + 2 + stand_wall + 4, rise]);
        }
        in_stand() {
            // Pocket for the shell and bezel, open at the top
            translate([-pocket_w / 2, 0, -fit])
                cube([pocket_w, stand_pocket + 100, pocket_d]);
            // Notch in front, wide enough for the reader
            translate([-(rdr_x / 2 + 3), 0, pocket_d - 1])
                cube([rdr_x + 6, stand_pocket + 100, reader_t + 40]);
        }
    }
}

// Printed flat on the bed.
module stand_print() { stand(); }

// Show the finished case sitting in the stand (for the assembly preview).
module stand_with_case() {
    color("tan") stand();
    in_stand() assembly_parts();
}
