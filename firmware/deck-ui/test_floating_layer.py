"""Bounded on-device UI check; no metrics feed and no application launcher."""
import argparse
import hashlib
import json
import re
import sys
import time
from pathlib import Path

import serial

parser = argparse.ArgumentParser()
parser.add_argument('--port', default='COM5')
parser.add_argument('--seconds', type=int, default=40)
args = parser.parse_args()
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
root = Path(__file__).resolve().parent
chunks = []
printed = set()
with serial.Serial(port=None, baudrate=115200, timeout=.05) as uart:
    uart.dtr = False
    uart.rts = False
    uart.port = args.port
    uart.open()
    uart.rts = True
    time.sleep(.1)
    uart.reset_input_buffer()
    uart.rts = False
    start = time.monotonic()
    sent = False
    while time.monotonic() - start < args.seconds:
        if not sent and time.monotonic() - start > 3:
            uart.write(b'K 1 12 11 10\nB\n')
            sent = True
        data = uart.read(min(4096, uart.in_waiting or 1))
        if data and sum(map(len, chunks)) < 2_000_000:
            chunks.append(data)
    raw = b''.join(chunks)
text = raw.decode('utf-8', errors='replace')
log = root / 'logs/floating-layer.log'
log.parent.mkdir(exist_ok=True)
log.write_bytes(raw)
for match in re.finditer(r'I \(\d+\) (?:deck-v4|deck-main): [^\r\n]+', text):
    line = match.group()
    if line not in printed:
        print(line[:230])
        printed.add(line)
layers = re.findall(r'floating layer page=(\d) top=(\d) passthrough=(\d) visible=(\d) slots=(\d)', text)
leaks = [int(v) for v in re.findall(r'overlay_leaks=(\d+)', text)]
fps = [float(v) for v in re.findall(r'render_fps=([\d.]+)', text)]
faults = [v for v in ['stack overflow', 'Guru Meditation', 'abort()', 'assert failed', 'allocation failed'] if v in text]
result = {
    'binary_sha256': hashlib.sha256((root/'build/pc_hub_deck.bin').read_bytes()).hexdigest(),
    'scope': 'Device boot, UART mapping and eight transitions; touch/optical checks require user observation',
    'mapping_saved': 'mapping configured 1/12/11/10 ESP_OK' in text,
    'pages_checked': sorted({int(row[0]) for row in layers}),
    'top_layer_and_passthrough': bool(layers) and all(row[1:3] == ('1', '1') for row in layers),
    'four_content_slots': bool(layers) and all(row[4] == '4' for row in layers),
    'home_only': bool(layers) and all(row[3] == '0' for row in layers if row[0] != '0'),
    'transition_overlay_leaks': leaks,
    'benchmark_complete': 'display benchmark complete' in text,
    'render_fps': fps,
    'touch_hide_observed': 'floating icons hidden' in text,
    'touch_show_observed': 'floating icons visible' in text,
    'voice_entry_observed': 'voice entry only; capture not started' in text,
    'faults': faults,
    'capture': False,
}
(root/'floating-layer-validation.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
print(json.dumps(result, ensure_ascii=False))
assert result['mapping_saved'] and result['pages_checked'] == list(range(5))
assert result['top_layer_and_passthrough'] and result['four_content_slots'] and result['home_only']
assert len(leaks) == 8 and not any(leaks)
assert result['benchmark_complete'] and not faults
print('PASS; serial closed; no background process installed')
