"""Regenerate the R3 interface-carrier draft, geometry and review drawings.

No guessed ESP32 development-board footprint or hub circuit is emitted.
The connector pin numbers describe our new harness, not the supplied modules.
"""
from pathlib import Path
import csv, json, math, uuid
from xml.etree.ElementTree import Element, SubElement, ElementTree
ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
OUT=ROOT/'hardware/pcb/exports/body-r3-review'
DRAW=ROOT/'design/drawings'
PARAM=ROOT/'hardware/enclosure/interface-parameters-r3.json'
for p in (HERE,OUT,DRAW,PARAM.parent): p.mkdir(parents=True,exist_ok=True)
P={
 'revision':'R3-NAPE-ADAPTATION-20261017','units':'mm','status':'review draft, NOT fabrication released',
 'body':{'width':228,'depth':76,'front_height':20,'angle_deg':5,'rear_height':round(20+76*math.tan(math.radians(5)),3),'wall':2.4,'corner_radius':5},
 'pcb':{'width':216,'depth':64,'thickness':1.6,'mount_holes':[[-104,-28],[104,-28],[104,28],[-104,28]],'hole_diameter':2.4},
 'screen':{'outline_diameter':44.32,'active_diameter':38.16,'thickness_budget':3.5,'rotates':False,'presses':True},
 'ring':{'outer_diameter':60,'upper_aperture':41.76,'lower_aperture':50,'carrier_diameter':49,'visible_border_per_side':1.8,'radial_clearance_per_side':0.5,'height_above_panel':8,'presses':False},
 'keys':{'count':4,'symmetric_x':True,'symmetric_y':True,'outer_x':108,'outer_y':31,'half_separator':1,'inner_arc_radius':34.5,'height_above_panel':0.3,'max_height_above_panel':1,'sensor_centers':[[-65,18],[65,18],[65,-18],[-65,-18]]},
 'contacts':{'sides':['left','right'],'face_x_abs':114,'y_positions':[-8.75,-6.25,-3.75,-1.25,1.25,3.75,6.25,8.75],'pitch':2.5,'magnet_y':[-27,27],'magnet_diameter':5,'magnets_carry_current':False,'signals':['5V_MAG_PROTECTED','GND','3V3_MAG_ISOLATED','MAG_SDA','MAG_SCL','MAG_DET','MAG_INT','GND'],'pin_order':'main-board face, rear to front; mating side mirrored','current_rating':'TBD by connector selection'},
 'accessories':{'macro':[112,76],'macro_keys':5,'macro_key_height':9,'joint_gap':1,'touch_strip':[104,22,8]},
 'rear_ports':[{'name':'USB-A1','x':-91},{'name':'USB-A2','x':-64},{'name':'USB-A3','x':64},{'name':'PC-C','x':91},{'name':'POWER-C','x':105}],
 'fast_port':{'side':'left','y':17},
 'microphone':{'part':'INMP441 module','voltage':3.3,'i2s':1,'bclk':4,'ws':5,'sd':17,'lr':'GND / left','hardware_tested':False,'acoustic_position':[-88,-38]},
 'central_keepout':{'radius':27,'purpose':'screen carrier, travel, FPC; no tall components'},
 'keyboard_plug':{'type':'adjustable USB-C male on independent carrier','fixed_pcb_plug':False},
}
PARAM.write_text(json.dumps(P,indent=2,ensure_ascii=False)+'\n',encoding='utf8')

# New-board logical harnesses: preserve the running firmware map, not ambiguous wire labels.
ports=[
 ('J1','MCU_DIGITAL_HARNESS',[ '3V3','GND','LCD_CS16','LCD_SCK6','LCD_D0_15','LCD_D1_7','LCD_D2_11','LCD_D3_12','TOUCH_SDA8','TOUCH_SCL9','KEY1_42','KEY2_41','KEY3_40','KEY4_38','PRESS39','ENC_A1','ENC_B2','MIC_BCLK4','MIC_WS5','MIC_SD17'],8,8),
 ('J2','MCU_POWER_AUDIO_HARNESS',['5V_BODY','GND','AUDIO_BCLK47','AUDIO_WS48','AUDIO_DOUT21','BOOT0','MAG_DET18','MAG_INT10','GND'],8,17),
 ('J3','LCD_QSPI_LOGICAL',['LCD_CS16','LCD_SCK6','LCD_D0_15','LCD_D1_7','LCD_D2_11','LCD_D3_12'],76,60),
 ('J4','LCD_TOUCH_POWER_LOGICAL',['3V3','GND','TOUCH_SDA8','TOUCH_SCL9'],101,60),
 ('J5','KEYS_AND_CENTRAL_PRESS',['GND','KEY1_42','KEY2_41','KEY3_40','KEY4_38','PRESS39'],48,45),
 ('J6','REMOTE_EC11_LOGICAL',['ENC_A1','GND','ENC_B2','GND','PRESS39'],147,31),
 ('J7','MAX98357A_MODULE',['5V_BODY','GND','AUDIO_BCLK47','AUDIO_WS48','AUDIO_DOUT21'],156,52),
 ('J8','INMP441_MODULE',['3V3','GND','MIC_BCLK4','MIC_WS5','MIC_SD17','GND'],166,10),
 ('J9','BODY_POWER_INPUT',['5V_BODY','GND'],197,56),
]
nets=sorted(set(n for _,_,ns,_,_ in ports for n in ns)|set(P['contacts']['signals']))
net_id={n:i+1 for i,n in enumerate(nets)}
serial=0
def uid():
    global serial
    serial+=1
    return str(uuid.uuid5(uuid.NAMESPACE_URL,f'KeyboardAsPCHub/R3/{serial}'))
def q(s):return json.dumps(str(s),ensure_ascii=False)
def footprint(ref,value,ns,x,y,vertical=False,smd=False):
    txt=[f'(footprint "R3:{ref}" (layer "F.Cu") (tstamp {uid()}) (at {x} {y})',
         f'(attr {"smd" if smd else "through_hole"})',
         f'(fp_text reference {q(ref)} (at 0 -2.6) (layer "F.SilkS") (effects (font (size 1 1) (thickness 0.15))))',
         f'(fp_text value {q(value)} (at 0 -4.5) (layer "F.Fab") (effects (font (size 0.8 0.8) (thickness 0.12))))']
    for i,net in enumerate(ns):
        px,py=(0,i*2.5) if vertical else (i*2.54,0)
        if smd:
            txt.append(f'(pad "{i+1}" smd roundrect (at {px} {py}) (size 2 1.7) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.2) (net {net_id[net]} {q(net)}))')
        else:
            txt.append(f'(pad "{i+1}" thru_hole {"rect" if i==0 else "circle"} (at {px} {py}) (size 1.7 1.7) (drill 0.9) (layers "*.Cu" "*.Mask") (net {net_id[net]} {q(net)}))')
    txt.append(')');return '\n'.join(txt)
board=['(kicad_pcb (version 20221018) (generator pcbnew)',
       '(general (thickness 1.6)) (paper "A4")',
       '(layers (0 "F.Cu" signal) (31 "B.Cu" signal) (36 "B.SilkS" user "b.silkscreen") (37 "F.SilkS" user "f.silkscreen") (44 "Edge.Cuts" user) (48 "B.Fab" user) (49 "F.Fab" user))',
       '(setup (pad_to_mask_clearance 0)) (net 0 "")']
for n,i in net_id.items():board.append(f'(net {i} {q(n)})')
for ref,value,ns,x,y in ports:board.append(footprint(ref,value,ns,x,y))
board.append(footprint('J10','LEFT_MAG_CONTACT_DRAFT',P['contacts']['signals'],2,23.25,True,True))
board.append(footprint('J11','RIGHT_MAG_CONTACT_DRAFT',P['contacts']['signals'],214,23.25,True,True))
for ref,(x,y) in enumerate([(4,4),(212,4),(212,60),(4,60)],1):
    board.append(f'(footprint "R3:M2_mount" (layer "F.Cu") (at {x} {y}) (attr through_hole) (fp_text reference "H{ref}" (at 0 2.5) (layer "F.SilkS") (effects (font (size 1 1) (thickness 0.15)))) (pad "" np_thru_hole circle (at 0 0) (size 2.4 2.4) (drill 2.4) (layers "*.Cu" "*.Mask")))')
for a,b in [((0,0),(216,0)),((216,0),(216,64)),((216,64),(0,64)),((0,64),(0,0))]:
    board.append(f'(gr_line (start {a[0]} {a[1]}) (end {b[0]} {b[1]}) (stroke (width 0.1) (type default)) (layer "Edge.Cuts"))')
board.append('(gr_circle (center 108 32) (end 135 32) (stroke (width 0.2) (type default)) (fill none) (layer "F.Fab"))')
for text,x,y,size in [('R3 INTERFACE CARRIER / UNROUTED',108,26,1.4),('NO GERBER RELEASE / HUB IS NOT IMPLEMENTED',108,38,1),('MIC OPTIONAL / 3V3 ONLY',180,19,1),('CENTRAL MECHANISM KEEP-OUT',108,32,1)]:
    board.append(f'(gr_text {q(text)} (at {x} {y}) (layer "F.SilkS") (effects (font (size {size} {size}) (thickness 0.15))))')
board.append(')')
(HERE/'body-r3.kicad_pcb').write_text('\n'.join(board)+'\n',encoding='utf8')
(HERE/'body-r3.kicad_pro').write_text(json.dumps({'meta':{'filename':'body-r3.kicad_pro','version':1}},indent=2)+'\n',encoding='utf8')

# Embedded passive connector symbols make the review schematic self-contained.
allports=ports+ [('J10','MAG_LEFT_DNP',P['contacts']['signals'],0,0),('J11','MAG_RIGHT_DNP',P['contacts']['signals'],0,0)]
sheet=uid(); sch=['(kicad_sch (version 20231120) (generator "eeschema")',f'(uuid {sheet}) (paper "A3")',
 '(title_block (title "Body R3 - module interface carrier") (rev "R3-review") (comment 1 "DRAFT: passive harness only; no hub, regulator or native USB routing"))','(lib_symbols']
for n in sorted(set(len(p[2]) for p in allports)):
    sch += [f'(symbol "R3:Conn_{n}" (pin_names (offset 1.016)) (in_bom yes) (on_board yes)',
            '(property "Reference" "J" (at 0 2.54 0) (effects (font (size 1.27 1.27))))',
            f'(property "Value" "Conn_{n}" (at 2.54 2.54 0) (effects (font (size 1.27 1.27))))',
            f'(symbol "Conn_{n}_1_1" (rectangle (start 0 1.27) (end 5.08 {-2.54*(n-1)-1.27}) (stroke (width 0.254) (type default)) (fill (type background)))']
    for i in range(n):
        sch.append(f'(pin passive line (at -2.54 {-2.54*i} 0) (length 2.54) (name "P{i+1}" (effects (font (size 1 1)))) (number "{i+1}" (effects (font (size 1 1)))))')
    sch.extend([')',')'])
sch.append(')')
for index,(ref,val,ns,_,_) in enumerate(allports):
    col,row=divmod(index,3);x=46+col*96;y=32+row*78
    sch.append(f'(symbol (lib_id "R3:Conn_{len(ns)}") (at {x} {y} 0) (unit 1) (in_bom yes) (on_board yes) (dnp {"yes" if ref in ("J10","J11") else "no"}) (uuid {uid()}) (property "Reference" {q(ref)} (at {x+3} {y-5} 0) (effects (font (size 1.27 1.27)))) (property "Value" {q(val)} (at {x+3} {y-2} 0) (effects (font (size 1 1))))')
    for i in range(len(ns)):sch.append(f'(pin "{i+1}" (uuid {uid()}))')
    sch.append(f'(instances (project "body-r3" (path "/{sheet}" (reference {q(ref)}) (unit 1)))))')
    for i,net in enumerate(ns):
        yy=y+2.54*i
        sch.append(f'(wire (pts (xy {x-15} {yy}) (xy {x-2.54} {yy})) (stroke (width 0) (type default)) (uuid {uid()}))')
        sch.append(f'(label {q(net)} (at {x-15} {yy} 0) (effects (font (size 1 1)) (justify left bottom)) (uuid {uid()}))')
sch.extend([f'(sheet_instances (path "/" (page "1")))',')'])
(HERE/'body-r3.kicad_sch').write_text('\n'.join(sch)+'\n',encoding='utf8')

with (OUT/'connector-map.csv').open('w',newline='',encoding='utf-8-sig') as f:
    w=csv.writer(f);w.writerow(['connector','interface','pin','net','status'])
    for ref,val,ns,_,_ in allports:
        for i,n in enumerate(ns):w.writerow([ref,val,i+1,n,'new harness definition; check actual module pinout'])
with (OUT/'bom-draft.csv').open('w',newline='',encoding='utf-8-sig') as f:
    w=csv.writer(f);w.writerow(['reference','qty','description','status'])
    for ref,val,ns,_,_ in ports:w.writerow([ref,1,f'1x{len(ns)} 2.54mm logical harness / {val}','footprint requires connector selection'])
    w.writerow(['J10,J11',2,'8-contact magnetic mating land pattern, 2.5mm pitch','DNP; protected power and I2C isolation not designed'])
    w.writerow(['H1-H4',4,'M2 clearance, drill 2.4mm','candidate coordinates'])
with (OUT/'unresolved-items.json').open('w',encoding='utf8') as f:
    json.dump({'fabrication_released':False,'routing_complete':False,'drc_run':False,'erc_run':False,'unknowns':['development-board exact model and footprint','screen backboard/FPC connector','final encoder/push mechanism','selected pogo connector/magnet/current rating','hub/PD/power-tree design','acoustic port and microphone module outline','physical GPIO2 vs GPIO42 wire-label discrepancy']},f,ensure_ascii=False,indent=2)

# Exact 1:1 logical outline SVG for review; screen circle and four mirrored key faces.
def svg_header(w,h):return f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}">'
def key_points(sx,sy):
    r=34.5;y0,y1=1,31;x0=math.sqrt(r*r-y0*y0);x1=math.sqrt(r*r-y1*y1)
    pts=[(108,y0),(108,y1),(x1,y1)]
    a0=math.asin(y1/r);a1=math.asin(y0/r)
    pts += [(r*math.cos(a0+(a1-a0)*i/32),r*math.sin(a0+(a1-a0)*i/32)) for i in range(33)]
    return [(sx*x,sy*y) for x,y in pts]
S=[svg_header(1240,670),'<rect width="1240" height="670" fill="#f4f6f7"/>','<g font-family="Segoe UI,Microsoft YaHei,sans-serif" fill="#192b35">',
 '<text x="48" y="42" font-size="25">R3 · Nape-inspired symmetrical body</text>',
 '<text x="48" y="69" font-size="14">228 × 76 mm · 5° · four mirrored keys · M1/M2 removed · review candidate</text>']
def px(x):return 610+x*4.4
def py(y):return 330-y*4.4
S.append(f'<rect x="{px(-114)}" y="{py(38)}" width="1003.2" height="334.4" rx="22" fill="#1e2b31" stroke="#51636b" stroke-width="2"/>')
for index,(sx,sy) in enumerate([(-1,1),(1,1),(1,-1),(-1,-1)],1):
    pts=' '.join(f'{px(x):.2f},{py(y):.2f}' for x,y in key_points(sx,sy))
    S.append(f'<polygon points="{pts}" fill="#d9e5e5" stroke="#87a5a8" stroke-width="1.5"/>')
    S.append(f'<text x="{px(sx*75)}" y="{py(sy*18)+5}" text-anchor="middle" font-size="18">{index:02}</text>')
S.append(f'<circle cx="610" cy="330" r="132" fill="#404d54" stroke="#7f9a9e" stroke-width="2"/>')
S.append(f'<circle cx="610" cy="330" r="91.872" fill="#0b1820" stroke="#87b4bd" stroke-width="2"/>')
S.append('<text x="610" y="324" fill="#dbfbff" text-anchor="middle" font-size="17">360 × 360</text><text x="610" y="349" fill="#a2c3cc" text-anchor="middle" font-size="13">touch + independent press</text>')
for side in [-1,1]:
    for y in P['contacts']['y_positions']:
        S.append(f'<rect x="{px(side*112)-3}" y="{py(y)-3}" width="6" height="6" rx="2" fill="#e6ba64"/>')
    for y in P['contacts']['magnet_y']:S.append(f'<circle cx="{px(side*111)}" cy="{py(y)}" r="8" fill="#71858c" stroke="#cad7dc"/>')
for p in P['rear_ports']:
    w=13 if p['name'].startswith('USB-A') else 9
    S.append(f'<rect x="{px(p["x"]-w/2)}" y="{py(38)-7}" width="{w*4.4}" height="13" rx="3" fill="#63838a"/>')
    S.append(f'<text x="{px(p["x"])}" y="{py(38)-16}" font-size="11" text-anchor="middle">{p["name"]}</text>')
S.append(f'<circle cx="{px(-88)}" cy="{py(-29)}" r="3" fill="#152833"/><text x="{px(-88)}" y="{py(-38)-2}" font-size="11" text-anchor="middle">MIC port / isolated chamber</text>')
S.extend(['<path d="M108 556H1112 M108 548V564 M1112 548V564" stroke="#617981"/>','<text x="610" y="583" text-anchor="middle" font-size="19">228 mm</text>',
 '<text x="48" y="618" font-size="14">Both side faces: 8 electrical contacts + 2 locating magnets. FAST-C remains on left rear side.</text>',
 '<text x="48" y="646" font-size="13">Ø60 ring / Ø44.32 panel / Ø38.16 active area / 1.8 mm visible border; not a fabrication drawing.</text></g></svg>'])
(DRAW/'layout-r3.svg').write_text(''.join(S),encoding='utf8')

B=[svg_header(1220,520),'<rect width="1220" height="520" fill="#f4f6f7"/><g font-family="Segoe UI,Microsoft YaHei,sans-serif" fill="#20323b">',
 '<text x="40" y="36" font-size="23">R3 PCB · 216 × 64 mm · module-interface carrier</text><text x="40" y="64" font-size="14">UNROUTED · module harness pin numbers are NEW definitions · no hub/PD circuit</text>',
 '<g transform="translate(70 100) scale(5)"><rect width="216" height="64" rx="1" fill="#234d45" stroke="#173b33" stroke-width="0.4"/>',
 '<circle cx="108" cy="32" r="27" fill="#dee5cf" opacity="0.3" stroke="#d4e9d0" stroke-dasharray="1 1" stroke-width="0.3"/>']
for ref,val,ns,x,y in ports:
    B.append(f'<text x="{x}" y="{y-3}" font-size="2.2" fill="white">{ref} {val}</text>')
    for i in range(len(ns)):B.append(f'<circle cx="{x+i*2.54}" cy="{y}" r="0.8" fill="#eac875"/><circle cx="{x+i*2.54}" cy="{y}" r="0.4" fill="#234d45"/>')
for x in [2,214]:
    for y in [23.25+i*2.5 for i in range(8)]:B.append(f'<rect x="{x-1}" y="{y-.85}" width="2" height="1.7" fill="#eac875"/>')
for x,y in [(4,4),(212,4),(212,60),(4,60)]:B.append(f'<circle cx="{x}" cy="{y}" r="1.2" fill="#f4f6f7"/>')
B.append('<text x="108" y="31" font-size="2.8" fill="white" text-anchor="middle">screen / press mechanism</text><text x="108" y="35" font-size="2.8" fill="white" text-anchor="middle">keep-out Ø54</text></g>')
B.append('<text x="40" y="462" font-size="14">No development-board socket is guessed. J1/J2 are cable harness inputs; verify all module pinouts.</text><text x="40" y="491" font-size="14">Magnetic pads are DNP: protection, current rating, alignment and I2C isolation remain to be designed.</text></g></svg>')
(OUT/'pcb-layout-r3.svg').write_text(''.join(B),encoding='utf8')
print('Generated R3 geometry, KiCad schematic/PCB draft, connector map, BOM and review SVGs.')
