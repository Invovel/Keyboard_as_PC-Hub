"""Draw a placement PROPOSAL from reviewed R3.1 envelopes; never writes KiCad."""
from pathlib import Path
from html import escape

HERE=Path(__file__).resolve().parent
W,H=1440,1200
S=5.2
def xy(x,y,cy):return 720+x*S,cy-y*S
items=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
 '<rect width="1440" height="1200" fill="#f6f8fb"/>',
 '<g font-family="Segoe UI,Arial,sans-serif" fill="#172d43">']
def text(x,y,value,size=15,color='#172d43',anchor='middle'):
    items.append(f'<text x="{x:.2f}" y="{y:.2f}" font-size="{size}" fill="{color}" text-anchor="{anchor}">{escape(value)}</text>')
def rect(x0,y0,x1,y1,cy,fill,stroke='#5b758a',dash='',radius=3):
    x,y=xy(x0,y1,cy)
    items.append(f'<rect x="{x:.2f}" y="{y:.2f}" width="{(x1-x0)*S:.2f}" height="{(y1-y0)*S:.2f}" rx="{radius}" fill="{fill}" stroke="{stroke}" stroke-width="1.4" stroke-dasharray="{dash}"/>')
def zone(box,cy,fill,lines,stroke='#5b758a',dash=''):
    x0,y0,x1,y1=box;rect(*box,cy,fill,stroke,dash)
    x,y=xy((x0+x1)/2,(y0+y1)/2,cy)
    for i,line in enumerate(lines):text(x,y+(i-(len(lines)-1)/2)*17+5,line,13)
def outline(cy):
    rect(-114,-38,114,38,cy,'#ffffff','#9faebe','',18)
    rect(-108,-32,108,32,cy,'#eef5f1','#3b725a','',10)
    for x in [-104,104]:
        for y in [-28,28]:
            px,py=xy(x,y,cy);items.append(f'<circle cx="{px}" cy="{py}" r="6.24" fill="#fff" stroke="#3b725a"/>')
    px,py=xy(0,0,cy)
    items.append(f'<circle cx="{px}" cy="{py}" r="{27*S}" fill="#fff4d9" stroke="#ac7b24" stroke-width="1.6" stroke-dasharray="8 5"/>')
    text(px,py-10,'R27 mechanical priority',16,'#876021')
    text(px,py+14,'no tall parts',14,'#876021')
    for side in [-1,1]:
        for y in [-8.75,-6.25,-3.75,-1.25,1.25,3.75,6.25,8.75]:
            px,py=xy(side*113,y,cy)
            items.append(f'<circle cx="{px}" cy="{py}" r="2.5" fill="#b5893b"/>')
        text(*xy(side*118,0,cy),'MAG',12,'#906523','end' if side<0 else 'start')
    for x,name in [(-91,'A1'),(-64,'A2'),(91,'PC-C / ONLY HOST')]:
        rect(x-5,35,x+5,39,cy,'#d6e7f7','#2b6796')
        text(*xy(x,42,cy),name,14,'#245f8c')
    rect(-115,12.5,-111,21.5,cy,'#d6e7f7','#2b6796')
    text(*xy(-118,17,cy),'FAST',13,'#245f8c','end')
text(55,34,'R3.1 CORE PCB — PLACEMENT PROPOSAL',27,anchor='start')
text(55,62,'Shared XY: screen centre = (0,0), rear = +Y. 216 x 64 mm electronics envelope; NOT a routed board.',16,anchor='start')
text(55,92,'A  |  TOP: major functional zones + projected upper key sensors',17,anchor='start')
cy=340
outline(cy)
rect(-25,28,78,32,cy,'#d8e9f6','#73a4c7','5 4')
text(*xy(26.5,29,cy),'USB routing reserve — feasibility TBD',12,'#245f8c')
zone((-56,2,-28,30),cy,'#cce5f8',['USB HUB','crystal / straps','try (-42,+16)'])
zone((-52,-14,-30,-4),cy,'#f6dbd6',['Hub rails','3V3 / VCORE'])
zone((32,-17,56,13),cy,'#d8e8ce',['ESP32-S3','local 3V3','debug / RF feed'])
zone((78,5,103,22),cy,'#f6dbd6',['5V select','PC / Dock','protect / isolate'])
zone((-52,-31,-28,-16),cy,'#e3dcf1',['ring encoder','mechanism study'])
zone((-102,-30,-80,-16),cy,'#cde9e5',['optional MIC','acoustic cavity'])
zone((34,-30,54,-20),cy,'#cde9e5',['I2S AMP'])
zone((80,-30,102,-12),cy,'#cde9e5',['speaker cavity','check magnetic','interference'])
rect(-24,-32,24,-27,cy,'#f7e2b6','#ac7b24','4 3')
text(*xy(0,-30.5,cy),'LCD / TOUCH connector study',12,'#876021')
for n,(x,y) in enumerate([(-65,18),(65,18),(65,-18),(-65,-18)],1):
    zone((x-7,y-7,x+7,y+7),cy,'#ffffffbb',[f'F{n}',f'({x},{y})'],'#6f8398','4 3')
text(720,568,'Dashed F1–F4: upper sensor-board projections, not free space on the main PCB.',16)
text(720,592,'RF feed may use an external-antenna module; the antenna itself needs a separate verified location.',15)
text(55,634,'B  |  BOTTOM: Dock functional reserves, projected in the SAME XY (not a mirrored fab view)',17,anchor='start')
cy=874
outline(cy)
zone((36,5,64,23),cy,'#dce4fa',['P4 DATA','USB3 + USB2'],'#626ba2','5 3')
zone((68,5,84,13),cy,'#f6dbd6',['PWR 5V'],'#b66559','5 3')
zone((68,15,84,23),cy,'#d8e8ce',['CTRL'],'#6b875a','5 3')
for x,y in [(-65,18),(65,18),(65,-18),(-65,-18)]:
    rect(x-7,y-7,x+7,y+7,cy,'none','#899bb1','4 4')
text(*xy(60,-5,cy),'48 x 18 mm study only',14,'#5d6594')
text(720,1105,'F2 and Dock overlap in XY: different Z levels REQUIRE connector, solder-tail and support checks.',16)
text(720,1133,'Core PCB Z = 6.4–8.0 mm; floor top Z = 2.4 mm. Nominal underside space = 4.0 mm, before tolerances.',15)
text(720,1162,'USB sockets need real footprint/face dimensions; the 6 mm shell-to-board setback may require local board tabs.',15)
items.extend(['</g>','</svg>'])
(HERE/'core-placement-r3-1.svg').write_text('\n'.join(items),encoding='utf-8')
print('Wrote placement proposal SVG; no KiCad files read or written.')
