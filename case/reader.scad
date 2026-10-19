// Mock-up of the HID ProxPoint Plus 6005, for fit checks only.
// Drawn in the case frame, sitting on the front face of the bezel.

module reader_mock() {
    color("gainsboro")
        translate([-rdr_x / 2, reader_cy - rdr_y / 2, Zi + front_t])
            cube([rdr_x, rdr_y, reader_t]);
    // 18" pigtail, shown as a short stub through the cable hole
    color("dimgray")
        translate([landscape ? reader_cable_dy : 0, reader_cy + (landscape ? 0 : reader_cable_dy), Zi - 10])
            cylinder(d = 5, h = 10 + front_t);
}

// The two screw positions in the case frame.
function reader_screws() =
    landscape ? [[-reader_screw_dy / 2, reader_cy], [reader_screw_dy / 2, reader_cy]]
              : [[0, reader_cy - reader_screw_dy / 2], [0, reader_cy + reader_screw_dy / 2]];
