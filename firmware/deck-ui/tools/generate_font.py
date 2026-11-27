"""Generate a local LVGL font from a user-supplied, appropriately licensed TTF."""
import argparse
from pathlib import Path
import shutil
import subprocess

p = argparse.ArgumentParser()
p.add_argument('--font', type=Path, required=True)
a = p.parse_args()
main = Path(__file__).resolve().parents[1] / 'main'
source = ''.join((main / f).read_text(encoding='utf-8') for f in ['deck_ui.c', 'deck_main.c'])
symbols = ''.join(sorted({c for c in source if ord(c) > 127}))
npx = shutil.which('npx.cmd') or shutil.which('npx')
if not npx:
    p.error('Install Node.js and npm first')
subprocess.run([npx, '--yes', 'lv_font_conv@1.5.3', '--font', str(a.font.resolve()),
    '--size', '14', '--bpp', '4', '--format', 'lvgl', '--range', '0x20-0x7e',
    '--symbols', symbols, '--lv-include', 'lvgl.h', '--lv-font-name', 'deck_font',
    '--lv-fallback', 'lv_font_montserrat_14', '--no-compress', '-o', str(main/'deck_font.c')], check=True)
