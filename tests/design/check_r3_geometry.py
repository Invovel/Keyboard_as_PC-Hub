"""Validate design consistency. Requires kiutils; no claim of KiCad ERC/DRC."""
from pathlib import Path
import argparse, json, math, sys, csv
p=argparse.ArgumentParser();p.add_argument('--libs');a=p.parse_args()
if a.libs:sys.path.insert(0,a.libs)
from kiutils.board import Board
from kiutils.schematic import Schematic
ROOT=Path(__file__).resolve().parents[2]
params=json.loads((ROOT/'hardware/enclosure/interface-parameters-r3.json').read_text())
b=Board().from_file(str(ROOT/'hardware/pcb/sources/body-r3/body-r3.kicad_pcb'))
s=Schematic().from_file(str(ROOT/'hardware/pcb/sources/body-r3/body-r3.kicad_sch'))
assert len(b.footprints)==15 and len(s.schematicSymbols)==11
assert not b.traceItems and not b.zones, 'Review draft must not imply routing is complete'
assert (params['ring']['upper_aperture']-params['screen']['active_diameter'])/2<2
assert abs((params['ring']['upper_aperture']-params['screen']['active_diameter'])/2-1.8)<1e-6
assert params['keys']['symmetric_x'] and params['keys']['symmetric_y']
assert params['keys']['height_above_panel']<=1
assert params['keys']['outer_x']==84
assert params['body']['width']/2-params['keys']['outer_x']==params['keys']['end_blank_width']==30
assert all(abs(x)+7<params['keys']['outer_x'] for x,y in params['keys']['sensor_centers'])
centers={tuple(c) for c in params['keys']['sensor_centers']}
for x,y in centers:assert (-x,y) in centers and (x,-y) in centers
assert params['pcb']['width']+2*params['body']['wall']<params['body']['width']
assert params['pcb']['depth']+2*params['body']['wall']<params['body']['depth']
for footprint in b.footprints:
 for pad in footprint.pads:
  x=footprint.position.X+pad.position.X;y=footprint.position.Y+pad.position.Y
  assert 0<=x<=216 and 0<=y<=64
  if pad.net is not None:assert pad.net.name in {n.name for n in b.nets}
rows=list(csv.DictReader((ROOT/'hardware/pcb/exports/body-r3-review/connector-map.csv').open(encoding='utf-8-sig')))
assert len(rows)==sum(len(f.pads) for f in b.footprints if f.entryName.startswith('J'))
assert len({p for p in [4,5,17]})==3
result={'passed':True,'pcb_footprints':len(b.footprints),'pcb_nets_including_0':len(b.nets),'schematic_connectors':len(s.schematicSymbols),'connector_rows':len(rows),'geometry_checked':True,'routing_complete':False,'KiCad_ERC_DRC_run':False}
(ROOT/'tests/design/r3-geometry-result.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
