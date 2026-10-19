"""Retired R3 generator. This entry point never writes KiCad or design files.

Original source is retained as generate-r3-legacy.py.txt for historical review.
Use hardware/enclosure/tools/update_r3_1.py for current JSON/SVG outputs.
Future KiCad changes must use Konnect, not this historical text generator.
"""
import sys

if __name__ == '__main__':
    print('R3 generator retired: no files written. Use hardware/enclosure/tools/update_r3_1.py; KiCad edits require Konnect.', file=sys.stderr)
    raise SystemExit(2)
