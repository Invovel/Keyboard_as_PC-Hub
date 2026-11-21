// RevC candidate geometry in product XYZ (millimetres). See docs/revc/ASSEMBLY.md.
// Use part = "assembly", "upper", or "lower". No mating holes use unknown hardware geometry.
part = "assembly";
show_upper = true;
show_envelopes = true;
$fn = 100;

width = 228;
depth = 76;
corner = 5;
wall = 2.4;
bottom = 2.4;
tilt = 5;
front_height = 20;
top_thickness = 2.4;
board_bottom = 6.4;
board_thickness = 1.6;
ring_opening = 62;
slot_width = 0.5;
slot_endpoints = [
    [[-31, 0], [-49, 0]], [[31, 0], [49, 0]],
    [[0, 31], [0, 37]], [[0, -31], [0, -37]]
];

function top_z(y) = front_height + (y + depth / 2) * tan(tilt);

module rounded(w, d, r) {
    offset(r = r) square([w - 2*r, d - 2*r], center = true);
}

module sloped_slab(thickness, z_offset = 0) {
    translate([0, 0, top_z(0) + z_offset])
        multmatrix([[1,0,0,0], [0,1,0,0], [0,tan(tilt),1,0], [0,0,0,1]])
            linear_extrude(thickness) children();
}

module lower() {
    difference() {
        intersection() {
            linear_extrude(35) rounded(width, depth, corner);
            sloped_slab(40, -40 - top_thickness) square([width + 2, depth + 2], center = true);
        }
        translate([0, 0, bottom]) linear_extrude(40)
            rounded(width - 2*wall, depth - 2*wall, corner - wall);
    }
    // Low bearing ribs for the initial study. Keep below the nominal PCB plane.
    translate([0, 0, bottom - 0.1]) linear_extrude(1.5)
        difference() { circle(d = 62); circle(d = 59.6); }
    for (x = [-108,101]) {
        translate([x, 34, bottom - 0.1]) cube([7, 4, 4], center = false);
    }
}

module slot(segment) {
    hull() {
        translate(segment[0]) circle(d = slot_width);
        translate(segment[1]) circle(d = slot_width / 2);
    }
}

module upper() {
    difference() {
        sloped_slab(top_thickness, -top_thickness) rounded(width, depth, corner);
        translate([0, 0, 15]) cylinder(d = ring_opening, h = 17);
        for (segment = slot_endpoints)
            translate([0, 0, 15]) linear_extrude(17) slot(segment);
        for (x = [65,68,71])
            translate([x, -33, 15]) linear_extrude(17) square([1,5], center = true);
    }
}

module board_envelope() {
    difference() {
        translate([-108, -32, board_bottom]) cube([216,64,board_thickness]);
        for (x = [-104,104], y = [-28,28])
            translate([x,y,board_bottom - 0.1]) cylinder(d = 2.4,h = board_thickness + 0.2);
    }
}

module envelopes() {
    color([0.08,0.55,0.4,0.55]) board_envelope();
    for (x = [-93,93])
        color([0.88,0.58,0.16,0.52]) translate([x,-3,12.5]) cube([15,54,6],center = true);
    // Ring and screen XY envelopes use display-only vertical offsets.
    color([0.65,0.65,0.7,0.5]) translate([0,0,top_z(0)])
        difference() { cylinder(d = 60,h = 1); cylinder(d = 58,h = 1); }
    color([0.15,0.35,0.65,0.5]) translate([0,0,top_z(0)])
        cylinder(d = 44.32,h = 1.8);
}

if (part == "lower" || part == "assembly") color("#293540") lower();
if (part == "upper" || (part == "assembly" && show_upper)) color([0.5,0.6,0.7,0.65]) upper();
if (part == "assembly" && show_envelopes) envelopes();
