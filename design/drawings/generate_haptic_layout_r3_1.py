"""Generate reference SVG/JSON only. Never writes any KiCad source file."""
import json
import math
from pathlib import Path
from html import escape

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
PARAMS = {
    "revision": "R3.1-HAPTIC-7PLUS-20261022",
    "status": "mechanical reservation and unrouted PCB placement study",
    "units": "mm",
    "body": [228, 76], "pcb": [216, 64, 1.6],
    "screen_origin": [0, 0],
    "kicad_origin": [150, 100],
    "kicad_mapping": "pcb_x=150+product_x; pcb_y=100-product_y",
    "top_blank_bands_x": [[-114, -84], [84, 114]],
    "motor_bays": [
        {"name": "LEFT", "xy": [-100.5, -30, -85.5, 24]},
        {"name": "RIGHT", "xy": [85.5, -30, 100.5, 24]},
    ],
    "motor_bay_size": [15, 54, 6],
    "motor_assembly_z": [9.5, 15.5],
    "pcb_bottom_z": 6.4, "pcb_top_z": 8.0,
    "minimum_nominal_roof_clearance": round(20 + 8 * math.tan(math.radians(5)) - 2.4 - 15.5, 3),
    "minimum_nominal_pcb_clearance": 1.5,
    "mount": "removable carrier attached to roof ribs; side bracket alternative; no final Apple hole pattern",
    "motor_candidates": ["iPhone 7 Plus Taptic Engine"],
    "motor_selection": {
        "target_device": "iPhone 7 Plus",
        "selection_locked_by_user": True,
        "dimension_source": "user-supplied design input on 2026-10-22; not independently measured or an Apple drawing",
        "body_bbox_mm": [43, 12.5, 2.6],
        "bbox_including_ears_mm": [50, 12.5, 2.6],
        "mass_g_user_input": 6.9,
        "previous_candidates": ["iPhone 13 Pro Taptic Engine", "iPhone 13 Pro Max Taptic Engine"],
        "previous_candidates_are_current_targets": False,
        "measured_motor_bbox_mm": None,
        "mounting_hole_coordinates_mm": None,
        "motor_fpc_connector": None,
        "drive_voltage_vrms": None,
        "drive_current_a": None,
        "resonance_hz": None,
        "body_length_axis_relative_to_product": "Y",
        "internal_vibration_axis_relative_to_product": None,
        "fit_gate": "user-supplied 50x12.5x2.6 bounding box fits 54x15x6 assembly target; holes, solder fillets, wire bend, actual sockets and mount still need verification",
        "purpose": "short haptic feedback only; no sensing or continuous audio",
        "sources": [
            "https://elchika.com/article/2bbe60e0-c923-434e-adc8-8bbf6bb0fd0e/",
            "https://www.ifixit.com/Guide/iPhone+7+Plus+Taptic+Engine+Replacement/67594",
            "https://www.ti.com/lit/ds/symlink/drv2605l.pdf"
        ]
    },
    "motor_fit_confirmed": False,
    "electrical_pinout_confirmed": False,
    "coil_connection_route": "public 7 Plus modification: two outermost pads of exposed six-test-pad row; project labels TP-A/TP-B, not Apple FPC pin numbers",
    "nominal_bbox_fit": True,
    "both_motors_populated_required": False,
    "driver": "DRV2605L integration target on PCB; optional external adapter via DNP JH headers; Apple-motor parameters not validated",
    "driver_connectors": [
        {"reference": "JH1", "xy": [-78, -6], "pins": 6},
        {"reference": "JH2", "xy": [78, -6], "pins": 6},
    ],
    "driver_interface_proposed": ["HAPTIC_3V3_PROTECTED", "GND", "SDA_3V3", "SCL_3V3", "EN_RESERVED", "NC"],
    "driver_interface_status": "proposed interface only; pads have no nets and must not be plugged into existing modules",
    "motor_output": "driver OUT+ to outer test pad TP-A; OUT- to opposite outer pad TP-B; inner four isolated; no output to ground",
    "two_driver_i2c": "same-address drivers need isolated channels or mux; not yet implemented",
    "mag_internal_headers_xy": [[-76.5, 6], [76.5, 6]],
    "power_selection_candidate_xy": [55, 2, 69, 10],
    "native_pcb": r"C:\Users\indeg\OneDrive\文档\KiCad\10.0\projects\Pc-Hub\Pc-Hub\Pc-Hub.kicad_pcb",
    "electrical_layout_complete": False,
}

def generate():
    out = ROOT / 'hardware/enclosure/haptic-reservation-r3-1.json'
    out.write_text(json.dumps(PARAMS, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="1440" height="1000" viewBox="0 0 1440 1000">',
           '<rect width="1440" height="1000" fill="#f4f7fa"/>',
           '<style>text{font-family:Segoe UI,Microsoft YaHei,sans-serif;fill:#23334a}.title{font-size:28px;font-weight:650}.sub{font-size:17px}.small{font-size:14px}</style>']
    def text(x,y,s,cls='sub',**attrs):
        a=' '.join(f'{k.replace("_","-")}="{v}"' for k,v in attrs.items())
        svg.append(f'<text x="{x}" y="{y}" class="{cls}" {a}>{escape(s)}</text>')
    def rect(x,y,w,h,fill,stroke='#496078',dash=None):
        d=f' stroke-dasharray="{dash}"' if dash else ''
        svg.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" fill="{fill}" stroke="{stroke}" stroke-width="1.5"{d}/>')
    def line(x1,y1,x2,y2,color='#496078'):
        svg.append(f'<path d="M{x1} {y1}L{x2} {y2}" fill="none" stroke="{color}" stroke-width="1.5"/>')
    text(48,52,'Core R3.1 · 左右马达预留与 PCB 布局','title')
    text(48,83,'锁定 iPhone 7 Plus；采用用户给定含耳 50×12.5×2.6 mm，沿 Y 放置；顶面保持完整留白。')
    ox,oy,scale=720,330,5
    def box(a,fill,stroke='#496078',dash=None):
        x0,y0,x1,y1=a
        rect(ox+x0*scale,oy-y1*scale,(x1-x0)*scale,(y1-y0)*scale,fill,stroke,dash)
    box([-114,-38,114,38],'#fff')
    box([-114,-38,-84,38],'#ecf5ed','#79a888')
    box([84,-38,114,38],'#ecf5ed','#79a888')
    box([-108,-32,108,32],'none','#234a70','6 4')
    for x in (-65,65):
        for y in (-18,18):
            box([x-7,y-7,x+7,y+7],'#e0e9f7','#7f9bbf')
            line(ox+(x-2)*scale,oy-y*scale,ox+(x+2)*scale,oy-y*scale)
            line(ox+x*scale,oy-(y-2)*scale,ox+x*scale,oy-(y+2)*scale)
    for r,color in [(30,'#294e79'),(27,'#9aa9ba'),(22.16,'#7f9bbf')]:
        svg.append(f'<circle cx="{ox}" cy="{oy}" r="{r*scale}" fill="none" stroke="{color}" stroke-width="1.5"/>')
    text(ox,oy-8,'圆屏 / 旋转环',text_anchor='middle')
    text(ox,oy+18,'中心固定',text_anchor='middle')
    for bay in PARAMS['motor_bays']:
        box(bay['xy'],'#f6dbad','#bb872e')
        xx=(bay['xy'][0]+bay['xy'][2])/2
        # Narrow end bays need vertical labels to avoid obscuring adjacent parts.
        tx,ty=ox+xx*scale,oy+15
        text(tx,ty,'7 Plus · 15 × 54',cls='small',text_anchor='middle',transform=f'rotate(-90 {tx} {ty})')
        box([xx-6.25,-28,xx+6.25,22],'none','#bb872e','3 2')
    for x in (-78,78):
        box([x-6,-9.3,x+6,-2.7],'#c5deed','#4084ab')
        text(ox+x*scale,oy+66,'驱动接口',cls='small',text_anchor='middle')
    box([-56,2,-28,30],'#e7e1f4','#9076b1')
    text(510,245,'Hub / 配置',cls='small',text_anchor='middle')
    box([34,-12,54,10],'#dce8f4','#5981aa')
    text(940,339,'ESP32-S3',cls='small',text_anchor='middle')
    box(PARAMS['power_selection_candidate_xy'],'#e7e1f4','#9076b1')
    text(1030,304,'电源候选',cls='small',text_anchor='middle')
    for x in (-104,104):
        for y in (-28,28):
            svg.append(f'<circle cx="{ox+x*scale}" cy="{oy-y*scale}" r="6" fill="#fff" stroke="#496078"/>')
    for x,name in [(-91,'A1'),(-64,'A2'),(64,'PC-C'),(91,'ISO')]:
        box([x-5,31,x+5,38],'#eff2f6','#6a7a90')
        text(ox+x*scale,oy-205,name,cls='small',text_anchor='middle')
    for x in (-114,114):
        for y in (-8.75,-6.25,-3.75,-1.25,1.25,3.75,6.25,8.75):
            svg.append(f'<circle cx="{ox+x*scale}" cy="{oy-y*scale}" r="3" fill="#a98130"/>')
    text(160,551,'外壳 228 × 76；PCB 216 × 64（虚线）；两端顶面各留白 30 mm。')
    text(160,578,'每侧装配区 15×54×6；X = ±85.5～±100.5；Y = −30～+24；四键及磁吸面不动。')
    text(48,634,'侧剖 · 默认安装方式：可拆支架固定到顶部内筋','title')
    # Schematic section: height scale exaggerated consistently, representative at bay front.
    sx,sy,k=260,900,10
    roof=20+8*math.tan(math.radians(5))
    rect(sx,sy-roof*k,920,24,'#c8d4df')
    text(1200,sy-roof*k+17,'完整顶面',cls='small')
    rect(sx+80,sy-15.5*k,220,6*k,'#f6dbad','#bb872e')
    text(sx+190,sy-12.5*k,'装配包络 6',cls='small',text_anchor='middle')
    line(sx+105,sy-(roof-2.4)*k,sx+105,sy-15.5*k)
    line(sx+275,sy-(roof-2.4)*k,sx+275,sy-15.5*k)
    rect(sx,sy-8*k,920,16,'#8bb8a6','#457b69')
    text(1200,sy-7*k,'PCB 1.6',cls='small')
    rect(sx,sy-2.4*k,920,24,'#c8d4df')
    text(1200,sy-1*k,'底壳 2.4',cls='small')
    text(sx+355,sy-15*k,'马达装配 Z = 9.5～15.5；顶部承力支架连接内筋。')
    text(sx+355,sy-12.5*k,'距 PCB 顶面名义 1.5 mm；Y=−30 最前端距顶壳约 2.80 mm。')
    text(sx+355,sy-10*k,'含耳长 50、宽 12.5、厚 2.6；不以机身外壳或马达照片反推孔距。')
    text(48,951,'状态：参考布局，未布线；尺寸来自用户输入。专用孔距、线束、驱动限幅及真实装配仍需验证。','sub')
    svg.append('</svg>')
    (HERE/'haptic-reservation-r3-1.svg').write_text('\n'.join(svg),encoding='utf-8')
    # Filled quadrilaterals are intentional: Konnect imports filled artwork as
    # native board graphics. This file is input artwork, never a PCB file.
    native = ['<svg xmlns="http://www.w3.org/2000/svg" width="228mm" height="76mm" viewBox="0 0 228 76">']
    def native_line(a,b,w=.12):
        x1,y1=114+a[0],38-a[1]
        x2,y2=114+b[0],38-b[1]
        length=math.hypot(x2-x1,y2-y1)
        dx,dy=-(y2-y1)*w/(2*length),(x2-x1)*w/(2*length)
        points=[(x1+dx,y1+dy),(x2+dx,y2+dy),(x2-dx,y2-dy),(x1-dx,y1-dy)]
        native.append('<polygon fill="#000" points="'+' '.join(f'{x:.5f},{y:.5f}' for x,y in points)+'"/>')
    def native_rect(x0,y0,x1,y1):
        p=[(x0,y0),(x1,y0),(x1,y1),(x0,y1)]
        for a,b in zip(p,p[1:]+p[:1]): native_line(a,b)
    def native_circle(x,y,r,steps=64):
        p=[(x+r*math.cos(i*2*math.pi/steps),y+r*math.sin(i*2*math.pi/steps)) for i in range(steps)]
        for a,b in zip(p,p[1:]+p[:1]): native_line(a,b)
    native_rect(-114,-38,114,38)
    for x in (-84,84): native_line((x,-38),(x,38))
    for bay in PARAMS['motor_bays']: native_rect(*bay['xy'])
    for radius in (30,27,22.16): native_circle(0,0,radius)
    for x in (-65,65):
        for y in (-18,18):
            native_rect(x-7,y-7,x+7,y+7)
            native_line((x-2,y),(x+2,y)); native_line((x,y-2),(x,y+2))
    for r in [(-56,2,-28,30),(-54,-18,-30,-10),(55,2,69,10),
              (-97,27,-85,38),(-70,27,-58,38),(85,27,97,38),
              (-114,13,-108,21),(36,5,64,23),(68,5,84,13),(68,15,84,23)]:
        native_rect(*r)
    for x in (-114,114):
        for y in (-8.75,-6.25,-3.75,-1.25,1.25,3.75,6.25,8.75):
            native_line((x-.8,y),(x+.8,y),.5)
        for y in (-27,27): native_circle(x,y,2.5,16)
    native.append('</svg>')
    (ROOT/'hardware/pcb/references/core-mechanical-overlay-r3-1.svg').write_text('\n'.join(native),encoding='utf-8')

if __name__ == '__main__':
    generate()
