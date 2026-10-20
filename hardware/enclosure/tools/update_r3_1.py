"""R3.1 mechanical/architecture outputs only. Never writes KiCad or historical R3.

Inherited R3 dimensions are candidates, not measured component footprints.
"""
from pathlib import Path
import json, math
ROOT=Path(__file__).resolve().parents[3]
BASE=ROOT/'hardware/enclosure/interface-parameters-r3.json'
OUT=ROOT/'hardware/enclosure/interface-parameters-r3-1.json'
DRAW=ROOT/'design/drawings'
p=json.loads(BASE.read_text(encoding='utf-8-sig'))
p['revision']='R3.1-CORE-OPTIONAL-DOCK-20261019'
p['status']='architecture and mechanical envelope study; NOT fabrication released'
p['product']={'name':'Desktop Control Hub','core_independent_required':True,'unique_host_upstream':'Core PC-C','power_dock_optional':True,'power_dock_revision':'B','internal_mains':False,'host_PD_charging':False}
p.pop('power_plan',None)
p['power']={'core_default_source':'PC-C VBUS, within detected and configured allowance','core_PD_high_voltage_input':False,'dock_to_core_direction':'one-way','pc_backfeed_allowed':False,'fixed_core_budget_w':None,'fixed_dock_budget_w':None,'hardware_verified':False}
p['rear_ports']=[{'name':'A1','x':-91,'logical_port':'P1'},{'name':'A2','x':-64,'logical_port':'P2'},{'name':'PC-C','x':91,'role':'only host upstream'}]
p['fast_port']={'name':'FAST-C','side':'left','y':17,'logical_port':'P3','data_target_Gbps':10,'source_v':5,'source_current_a':None}
p['logical_downstream']={'P1':'A1','P2':'A2','P3':'FAST-C','P4':'DOCK DATA','P5':'KBD USB2','P6':'CTRL USB2 FS HID'}
p['dock']={'revision':'B','present_by_default':False,'second_host_upstream':False,'connector_part_number':None,'connector_frozen':False,'study_only':True,'zone_xy':[36,5,84,23],'data_zone_xy':[36,5,64,23],'pwr_zone_xy':[68,5,84,13],'ctrl_zone_xy':[68,15,84,23],'core_pcb_bottom_z':6.4,'dock_pcb_top_z_study':-1.6,'board_spacing_study':8,'body_height_study':20,'body_top_z_study':-1,'power_direction':'Dock to Core only','data_via':'Core P4 downstream','power_rating_a':None,'high_speed_on_MAG':False}
p['accessories'].update({'layout':'five keys in one horizontal row; K5 Fn target','slider_position':'front of five-key module, horizontal only','slider_active_length_target':[90,96]})
p['accessories'].update({'slider_active_width_target':[12,16],'slider_update_hz_target':100,'slider_position_bits_target':8,'slider_default_mode':'relative','slider_tested':False})
p['total_disable']={'position':'front-center','candidate_xy':[0,-38],'runtime_GPIO':0,'switch_part_number':None}
OUT.write_text(json.dumps(p,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

def begin(w,h):
    return [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}">',f'<rect width="{w}" height="{h}" fill="#f4f6f7"/>','<g font-family="Segoe UI,Microsoft YaHei,sans-serif" fill="#20323b">']
def text(x,y,s,size=15):return f'<text x="{x}" y="{y}" font-size="{size}">{s}</text>'
S=begin(1240,670)
S += [text(48,42,'R3.1 · Desktop Control Hub / Core + optional Power Dock',25),text(48,69,'228 × 76 mm · 5° · 30 mm end lands · candidate dimensions / no manufacturing release',14)]
px=lambda x:610+x*4.4
py=lambda y:330-y*4.4
S.append(f'<rect x="{px(-114)}" y="{py(38)}" width="1003.2" height="334.4" rx="22" fill="#1e2b31"/>')
for i,(sx,sy) in enumerate([(-1,1),(1,1),(1,-1),(-1,-1)],1):
    r=p['keys']['inner_arc_radius']; a=math.asin(31/r); b=math.asin(1/r);w=p['keys']['outer_x']
    q=[(w,1),(w,31),(math.sqrt(r*r-31*31),31)]+[(r*math.cos(a+(b-a)*j/32),r*math.sin(a+(b-a)*j/32)) for j in range(33)]
    points=' '.join(f'{px(sx*x):.2f},{py(sy*y):.2f}' for x,y in q)
    S.append(f'<polygon points="{points}" fill="#d9e5e5" stroke="#87a5a8" stroke-width="1.5"/>')
    S.append(f'<text x="{px(sx*65)}" y="{py(sy*18)+5}" text-anchor="middle">{i:02}</text>')
for side in [-1,1]:
    S.append(f'<text x="{px(side*99)}" y="335" text-anchor="middle" fill="#a2c3cc">30 mm</text>')
    for y in p['contacts']['y_positions']:S.append(f'<rect x="{px(side*112)-3}" y="{py(y)-3}" width="6" height="6" rx="2" fill="#e6ba64"/>')
    for y in p['contacts']['magnet_y']:S.append(f'<circle cx="{px(side*111)}" cy="{py(y)}" r="8" fill="#71858c"/>')
for port in p['rear_ports']:
    w=13 if port['name'].startswith('A') else 9
    S.append(f'<rect x="{px(port["x"]-w/2)}" y="{py(38)-7}" width="{w*4.4}" height="13" rx="3" fill="#63838a"/>')
    S.append(f'<text x="{px(port["x"])}" y="{py(38)-16}" text-anchor="middle" font-size="13">{port["name"]}</text>')
S += [f'<circle cx="610" cy="330" r="132" fill="#404d54" stroke="#7f9a9e" stroke-width="2"/>',f'<circle cx="610" cy="330" r="91.872" fill="#0b1820" stroke="#87b4bd" stroke-width="2"/>','<text x="610" y="326" fill="#dbfbff" text-anchor="middle" font-size="18">360 × 360</text><text x="610" y="351" fill="#a2c3cc" text-anchor="middle">touch + press</text>']
S.append(f'<rect x="{px(-114)-5}" y="{py(21.5)}" width="10" height="39.6" rx="3" fill="#63838a"/>')
S.append(f'<rect x="{px(-5)}" y="{py(-38)-4}" width="44" height="8" rx="2" fill="#c4aa7f"/><text x="610" y="528" text-anchor="middle" font-size="13">Disable / front center (candidate)</text>')
S += [text(48,555,'Rear: A1 + A2 + PC-C (only host upstream). Left: FAST-C. Bottom: P4 Dock reserve.',16),text(48,586,'No Core A3 or POWER IN. External PD moves to optional lower Rev B Dock.',16),text(48,617,'Five-key module + one front horizontal 104 mm slider retained. MAG: low power / low speed.',14),text(48,646,'Ring Ø60 / panel Ø44.32 / AA Ø38.16; controller, GPIO and prior test records unchanged.',14),'</g></svg>']
(DRAW/'layout-r3-1.svg').write_text(''.join(S),encoding='utf8')

S=begin(1240,830)
S += [text(38,38,'R3.1 Dock interface: underside projection + side section / STUDY ONLY',24),text(38,68,'Functional zones are not connector footprints. Pinout, part numbers, current rating and openings are TBD.',15)]
# Underside projected onto the same X/Y axes as the top view, rather than a mirrored mating drawing.
sx=lambda x:600+x*4.4
sy=lambda y:285-y*4.4
S.append(f'<rect x="{sx(-114)}" y="{sy(38)}" width="1003.2" height="334.4" rx="22" fill="#e1e7e8" stroke="#5f767f"/>')
S.append(f'<circle cx="600" cy="285" r="118.8" fill="none" stroke="#718995" stroke-dasharray="6 5"/>')
for x in [-104,104]:
    for y in [-28,28]:S.append(f'<circle cx="{sx(x)}" cy="{sy(y)}" r="5.3" fill="white" stroke="#617981"/>')
for key,label,color in [('data_zone_xy','DATA 28×18','#9bbcc3'),('pwr_zone_xy','PWR 16×8','#dac59c'),('ctrl_zone_xy','CTRL 16×8','#adc3a3')]:
    x0,y0,x1,y1=p['dock'][key]
    S.append(f'<rect x="{sx(x0)}" y="{sy(y1)}" width="{(x1-x0)*4.4}" height="{(y1-y0)*4.4}" fill="{color}" stroke="#526b75" stroke-dasharray="5 3"/>')
    S.append(f'<text x="{sx((x0+x1)/2)}" y="{sy((y0+y1)/2)+4}" text-anchor="middle" font-size="11">{label}</text>')
S += [text(44,482,'Same X/Y axes as top: X right, Y rear. Zone: X +36…+84 / Y +5…+23 = 48 × 18 mm.',15),text(44,510,'Nominal separation only: center keep-out R27 and mounting holes are preserved; actual 3D clearance TBD.',14)]
# Schematic side section at the Dock zone; connector walls intentionally not a real component model.
S += [text(44,558,'Side-section study (Z mm)',19),'<rect x="110" y="585" width="730" height="10" fill="#789983"/>',text(870,596,'Core PCB bottom Z = +6.4',15),'<rect x="110" y="617" width="730" height="12" fill="#7a8d98"/>',text(870,629,'Core base Z = 0…2.4',15),'<rect x="414" y="595" width="110" height="41" fill="#d7e1e5" stroke="#527380" stroke-dasharray="5 3"/>','<rect x="110" y="636" width="730" height="8" fill="#789983"/>',text(870,646,'Dock PCB top Z = −1.6',15),'<rect x="100" y="633" width="750" height="102" rx="8" fill="none" stroke="#527380" stroke-dasharray="7 4"/>',text(870,680,'Dock case: −21…−1 (20 high)',15),text(130,697,'Board-plane spacing 8 mm; window through Core base is NOT designed yet.',15),text(44,780,'Rev A default: no Dock. Rev B: P4 downstream DATA + one-way 5V PWR + low-speed CTRL; no second PC uplink.',14),text(44,807,'Guide/retention and enclosure load paths must be designed after the actual mating connectors are selected.',14),'</g></svg>']
(DRAW/'dock-interface-r3-1.svg').write_text(''.join(S),encoding='utf8')
print('Generated R3.1 JSON and two SVG studies; historical R3 and KiCad files untouched.')
