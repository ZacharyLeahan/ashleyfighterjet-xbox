#!/usr/bin/env python3
"""Launch an isolated 64 MB Xbox; keep all mutable state outside the repo."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--bootrom', type=Path, default=os.environ.get('XBOX_MCPX'))
parser.add_argument('--bios', type=Path, default=os.environ.get('XBOX_BIOS'))
parser.add_argument('--hdd', type=Path, default=os.environ.get('XBOX_HDD'))
parser.add_argument('--xemu', type=Path, default=Path('/Applications/Xemu.app/Contents/MacOS/xemu'))
parser.add_argument('--iso', type=Path, default=root/'build/game.iso')
parser.add_argument('--state-dir', type=Path, default=Path.home()/'Library/Application Support/AshleyFighterJet/xemu-test')
parser.add_argument('--check', action='store_true', help='Check inputs without launching')
a = parser.parse_args()
missing = [name for name in ('bootrom', 'bios', 'hdd') if getattr(a, name) is None or not getattr(a, name).is_file()]
if missing:
    sys.exit('Missing external Xbox files: '+', '.join(missing)+'. Supply --bootrom, --bios and --hdd (see README).')
if a.bootrom.stat().st_size != 512:
    sys.exit('MCPX boot ROM must be 512 bytes.')
for path in (a.xemu, a.iso):
    if not path.is_file(): sys.exit('Missing file: '+str(path))
if a.check:
    print('Inputs present. Boot and gameplay still require verification in xemu.'); sys.exit(0)
a.state_dir.mkdir(parents=True, exist_ok=True)
config = a.state_dir/'xemu.toml'
# JSON basic strings are also valid TOML basic strings for ordinary filesystem paths.
quote = lambda path: json.dumps(str(path.resolve()))
config.write_text('[general]\nshow_welcome = false\n[sys]\nmem_limit = "64"\n[sys.files]\n'+
                  '\n'.join(key+' = '+quote(value) for key,value in {
                      'bootrom_path':a.bootrom,'flashrom_path':a.bios,'hdd_path':a.hdd,
                      'eeprom_path':a.state_dir/'eeprom.bin'}.items())+'\n')
command = [str(a.xemu), '-config_path', str(config), '-dvd_path', str(a.iso), '-snapshot']
print('Launching 64 MB Xbox. HDD writes are discarded; configuration and EEPROM stay in '+str(a.state_dir))
print('Log: '+str(a.state_dir/'xemu.log'))
with (a.state_dir/'xemu.log').open('w') as log:
    result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
sys.exit(result.returncode)
