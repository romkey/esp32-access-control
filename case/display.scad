// Mock-up of the SPI TFT module, for fit checks only.
// Drawn in the display frame: centred, +Y toward the header end, and
// Z = 0 at the inside face of the bezel plate with the display behind it (-Z).

module display_mock() {
    z_pcb_front = -disp_gap - disp_stack;
    // Glass and touch panel
    color("black")
        translate([-disp_bl[0] / 2, -disp_l / 2 + disp_bl_off, z_pcb_front])
            cube([disp_bl[0], disp_bl[1], disp_stack]);
    // Lit area, drawn just proud of the glass so it is visible in previews
    color("royalblue")
        translate([-disp_aa[0] / 2, -disp_l / 2 + disp_aa_off, -disp_gap - 0.05])
            cube([disp_aa[0], disp_aa[1], 0.05]);
    // PCB
    color("darkgreen") difference() {
        translate([-disp_w / 2, -disp_l / 2, z_pcb_front - disp_pcb_t])
            cube([disp_w, disp_l, disp_pcb_t]);
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * disp_hole_dx / 2, sy * disp_hole_dy / 2, z_pcb_front - disp_pcb_t - 1])
                cylinder(d = disp_hole_d, h = disp_pcb_t + 2);
    }
    // 14-pin header, on the back, pins pointing away from the glass
    color("dimgray")
        translate([-disp_hdr_len / 2 - 1.27, disp_l / 2 - disp_hdr_in - 1.27, -disp_gap - disp_total])
            cube([disp_hdr_len + 2.54, 2.54, disp_total - disp_stack - disp_pcb_t]);
    // Approximate footprint of the SD socket and other back-side parts
    color("silver")
        translate([-10, -disp_l / 2 + 25, z_pcb_front - disp_pcb_t - disp_smd])
            cube([22, 30, disp_smd]);
}
