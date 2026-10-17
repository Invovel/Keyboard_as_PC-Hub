"""Bounded serial capture for bring-up; writes raw logs and closes the port."""
import argparse
import codecs
import sys
import time
from pathlib import Path
import serial

p = argparse.ArgumentParser()
p.add_argument('--port', default='COM5')
p.add_argument('--seconds', type=int, default=180)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--reset', action='store_true')
a = p.parse_args()
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
decoder = codecs.getincrementaldecoder('utf-8')(errors='replace')
a.output.parent.mkdir(parents=True, exist_ok=True)
with serial.Serial(port=None, baudrate=115200, timeout=0.2) as uart:
    uart.dtr = False
    uart.rts = False
    uart.port = a.port
    uart.open()
    if a.reset:
        uart.rts = True
        time.sleep(0.1)
        uart.reset_input_buffer()
        uart.rts = False
    until = time.monotonic() + a.seconds
    with a.output.open('wb') as out:
        print(f'Capturing {a.port} for {a.seconds}s to {a.output}', flush=True)
        try:
            while time.monotonic() < until:
                data = uart.read(uart.in_waiting or 1)
                if data:
                    out.write(data)
                    out.flush()
                    print(decoder.decode(data), end='', flush=True)
        except KeyboardInterrupt:
            pass
