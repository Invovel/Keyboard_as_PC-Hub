// R3 study model, mm. Not released for manufacture; no verified bearing/FPC stack.
// The matching dimensional source is interface-parameters-r3.json.
// Select part="assembly", "bottom", "top", "ring", "carrier", "keys", "pcb".
part="assembly";
explode=0;             // Display separation only, not a physical dimension.
W=228; D=76; wall=2.4; base=2.4; angle=5; front_h=20;
key_outer_x=84;        // Each key ends 30 mm before its body end; sensors stay at +/-65.
$fn=96;
function topz(y)=front_h+(y+D/2)*tan(angle);
module round_outline(w,d,r=5){offset(r=r) square([w-2*r,d-2*r],center=true);}
module solid_case(){
 intersection(){
  linear_extrude(40)round_outline(W,D);
  translate([0,0,topz(0)])rotate([angle,0,0])translate([-150,-80,-80])cube([300,160,80]);
 }
}
module contact_cutouts(){
 for(s=[-1,1]){
  translate([s*(W/2-wall/2),0,11])cube([wall+2,24,5],center=true);
  for(y=[-27,27])translate([s*(W/2-wall/2),y,11])rotate([0,90,0])cylinder(d=5.2,h=wall+1,center=true);
 }
}
module port_cutouts(){
 // Blank these openings until an actual hub/PD daughterboard is selected.
 for(x=[-91,-64,64])translate([x,D/2,11])cube([14,8,7],center=true);
 for(x=[91,105])translate([x,D/2,11])cube([9,8,4],center=true);
 translate([-W/2,17,11])cube([8,9,4],center=true);
 // Front-wall acoustic duct; microphone itself must be gasketed behind this bore.
 translate([-88,-D/2,9])rotate([90,0,0])cylinder(d=1.5,h=8,center=true);
}
module bottom(){
 difference(){
  solid_case();
  translate([0,0,base])linear_extrude(40)round_outline(W-2*wall,D-2*wall,3);
  contact_cutouts();port_cutouts();
 }
 // PCB mounting bosses: keep axis and height consistent with the layout study.
 for(x=[-104,104],y=[-28,28])translate([x,y,base])difference(){cylinder(d=6,h=4);cylinder(d=1.6,h=5);}
}
module face_key(sx,sy){
 difference(){
  translate([sx*(key_outer_x+1)/2,sy*16])square([key_outer_x-1,30],center=true);
  circle(r=34.5);
 }
}
module top(){
 difference(){
  translate([0,0,topz(0)-2.4])rotate([angle,0,0])linear_extrude(2.4)round_outline(W,D);
  for(sx=[-1,1],sy=[-1,1])translate([0,0,14])linear_extrude(24)offset(delta=.35)face_key(sx,sy);
  translate([0,0,14])cylinder(d=61,h=25);
 }
}
module ring(){
 // Display representation of stepped ring: lips, bearings and anti-lift fasteners need engineering.
 difference(){cylinder(d=60,h=8);translate([0,0,-.1])cylinder(d=50,h=7.1);translate([0,0,6.9])cylinder(d=41.76,h=2);}
}
module carrier(){
 difference(){cylinder(d=49,h=2);translate([0,0,.8])cylinder(d=45,h=2);}
 // Three peripheral supports carry the frame, not the glass.
 for(a=[0,120,240])rotate([0,0,a])translate([23.4,0,-3])cylinder(d=2.2,h=3);
}
module keys(){
 for(sx=[-1,1],sy=[-1,1])translate([0,0,topz(0)+.3-1.6])rotate([angle,0,0])linear_extrude(1.6)face_key(sx,sy);
}
module pcb(){difference(){translate([-108,-32,6.4])cube([216,64,1.6]);for(x=[-104,104],y=[-28,28])translate([x,y,6])cylinder(d=2.4,h=3);}}
module assembly(){
 color("#34454b")bottom();
 color("#6b8e72")translate([0,0,explode*.35])pcb();
 color("#26363d")translate([0,0,explode])top();
 color("#bed4d3")translate([0,0,explode])keys();
 translate([0,0,topz(0)+explode])rotate([angle,0,0]){
  color("#53636b")ring();
  color("#9cacb0")translate([0,0,1.9])carrier();
  color("#152d3c")translate([0,0,2.7])cylinder(d=44.32,h=3.5);
 }
}
if(part=="assembly")assembly();
else if(part=="bottom")bottom();
else if(part=="top")top();
else if(part=="ring")ring();
else if(part=="carrier")carrier();
else if(part=="keys")keys();
else if(part=="pcb")pcb();
