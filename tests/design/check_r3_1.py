"""R3.1 architecture consistency; not electrical, USB compliance, or CAD verification."""
from pathlib import Path
import csv, hashlib, json, subprocess, sys
from xml.etree import ElementTree as ET

ROOT=Path(__file__).resolve().parents[2]
p=json.loads((ROOT/'hardware/enclosure/interface-parameters-r3-1.json').read_text())
assert p['product']['unique_host_upstream']=='Core PC-C'
assert p['product']['core_independent_required'] and p['product']['power_dock_optional']
assert p['power']['core_PD_high_voltage_input'] is False
assert p['power']['pc_backfeed_allowed'] is False
assert p['power']['fixed_core_budget_w'] is None and p['power']['fixed_dock_budget_w'] is None
assert [v['name'] for v in p['rear_ports']]==['A1','A2','PC-C']
assert p['logical_downstream']=={'P1':'A1','P2':'A2','P3':'FAST-C','P4':'DOCK DATA','P5':'KBD USB2','P6':'CTRL USB2 FS HID'}
assert p['keys']['outer_x']==84 and p['keys']['end_blank_width']==30
assert p['keys']['sensor_centers']==[[-65,18],[65,18],[65,-18],[-65,-18]]
assert p['accessories']['macro_keys']==5 and p['accessories']['touch_strip']==[104,22,8]
assert p['total_disable']['position']=='front-center' and p['total_disable']['runtime_GPIO']==0
d=p['dock']
assert not d['present_by_default'] and not d['second_host_upstream'] and not d['connector_frozen']
assert d['connector_part_number'] is None and d['power_direction']=='Dock to Core only'
assert d['core_pcb_bottom_z']-d['dock_pcb_top_z_study']==d['board_spacing_study']==8
x0,y0,x1,y1=d['zone_xy']; assert [x1-x0,y1-y0]==[48,18]
assert x0>p['central_keepout']['radius']
for x,y in p['pcb']['mount_holes']:
    assert not (x0-3<=x<=x1+3 and y0-3<=y<=y1+3)
zones=[d[k] for k in ['data_zone_xy','pwr_zone_xy','ctrl_zone_xy']]
for a,b,c,e in zones:assert x0<=a<c<=x1 and y0<=b<e<=y1
for i,a in enumerate(zones):
    for b in zones[i+1:]:assert min(a[2],b[2])<=max(a[0],b[0]) or min(a[3],b[3])<=max(a[1],b[1])
rows=list(csv.DictReader((ROOT/'hardware/electronics/dock-port-selection.csv').open(encoding='utf-8-sig')))
mapping={r['logical_port']:r['port'] for r in rows if r['logical_port'].startswith('P')}
assert mapping=={'P1':'A1','P2':'A2','P3':'FAST-C','P4':'DOCK_DATA','P5':'KBD','P6':'CTRL'}
assert next(r for r in rows if r['port']=='PD_IN')['location']=='Dock_only'
for name in ['layout-r3-1.svg','dock-interface-r3-1.svg']:ET.parse(ROOT/'design/drawings'/name)
manifest=json.loads((ROOT/'docs/r3-1-package-source.json').read_text(encoding='utf-8-sig'))
for name,sha in manifest['imported_file_sha256'].items():assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==sha
ref=ROOT/'docs/references/r3-1-discussion'
for entry in json.loads((ref/'sources.json').read_text(encoding='utf-8-sig')):
    assert hashlib.sha256((ref/entry['file']).read_bytes()).hexdigest()==entry['sha256']
legacy=subprocess.run([sys.executable,str(ROOT/'hardware/pcb/sources/body-r3/generate.py')],cwd=ROOT,capture_output=True,text=True)
assert legacy.returncode==2 and 'no files written' in legacy.stderr
protected=['firmware','releases','tests/hardware','hardware/enclosure/interface-parameters-r3.json','hardware/enclosure/sources/body-r3.scad','design/drawings/layout-r3.svg']
protected += [str(v.relative_to(ROOT)).replace('\\','/') for v in (ROOT/'hardware/pcb').rglob('*.kicad_*')]
changed=subprocess.check_output(['git','diff',manifest['baseline_commit'],'--name-only','--',*protected],cwd=ROOT,text=True).strip()
assert not changed, f'Protected historical/code files changed: {changed}'
result={'date':'2026-10-19','architecture_consistency':True,'import_integrity':True,'legacy_generator_refuses_writes':True,'preserved_firmware_releases_logs_r3_geometry_and_kicad':True,'candidate_zone_boundary_check':True,'physical_hardware_tested':False,'KiCad_ERC_DRC_run':False,'OpenSCAD_compiled':False,'manufacturing_released':False}
(ROOT/'tests/design/r3-1-result.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
