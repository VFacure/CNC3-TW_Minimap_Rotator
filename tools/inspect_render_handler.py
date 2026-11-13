"""Read-only PE cross references to minimap interface strings."""
from pathlib import Path
import sys,struct,json
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools/python_deps'))
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
p=Path(r'D:\SteamLibrary\steamapps\common\Command and Conquer 3 Tiberium Wars\RetailExe\1.9\cnc3game.dat')
b=p.read_bytes();pe=pefile.PE(data=b);base=pe.OPTIONAL_HEADER.ImageBase;md=Cs(CS_ARCH_X86,CS_MODE_32)
lines=[]
for term in [b'Render']:
 pos=b.find(term+b'\0')
 if pos<0:
  lines.append(f'STRING {term.decode()} not present as null-terminated ASCII');continue
 va=base+pe.get_rva_from_offset(pos);needle=struct.pack('<I',va)
 lines.append(f'\nSTRING {term.decode()} VA={va:08x}')
 for sec in pe.sections:
  if not sec.Characteristics&0x20000000:continue
  data=sec.get_data();start=0
  while (i:=data.find(needle,start))>=0:
   start=i+1; at=base+sec.VirtualAddress+i; lines.append(f'REF {at:08x}')
   # Start at a nearby INT3 boundary when available, avoiding arbitrary instruction alignment.
   lo=max(0,i-120);cut=data.rfind(b'\xcc',lo,i);lo=cut+1 if cut>=0 else max(0,i-16)
   for ins in md.disasm(data[lo:i+100],base+sec.VirtualAddress+lo):lines.append(f'{ins.address:08x} {ins.mnemonic:8} {ins.op_str}')
(ROOT/'reports/render-handler-xrefs.txt').write_text('\n'.join(lines));print('\n'.join(lines))



