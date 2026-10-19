// Mock-up of the HID ProxPoint Plus 6005, for fit checks only.
// Drawn in the case frame, sitting on the front face of the bezel.

module reader_mock() {
    color("gainsboro")
        translate([-reader_w / 2, reader_cy - reader_h / 2, Zi + front_t])
            cube([reader_w, reader_h, reader_t]);
    // 18" pigtail, shown as a short stub through the cable hole
    color("dimgray")
        translate([0, reader_cy + reader_cable_dy, Zi - 10])
            cylinder(d = 5, h = 10 + front_t);
}

// The two screw positions in the case frame.
function reader_screws() = [[0, reader_cy - reader_screw_dy / 2],
                            [0, reader_cy + reader_screw_dy / 2]];
