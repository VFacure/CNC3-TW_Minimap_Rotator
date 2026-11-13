from pathlib import Path
import sys,struct
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools/python_deps'))
import pefile
from capstone import *
p=pefile.PE(r'D:\SteamLibrary\steamapps\common\Command and Conquer 3 Tiberium Wars\RetailExe\1.9\cnc3game.dat');b=p.get_memory_mapped_image();base=p.OPTIONAL_HEADER.ImageBase;md=Cs(CS_ARCH_X86,CS_MODE_32)
targets={0x8bed70};out=[]
for sec in p.sections:
 if not sec.Characteristics&0x20000000:continue
 d=sec.get_data();va=base+sec.VirtualAddress
 for i in range(len(d)-5):
  if d[i]!=0xe8:continue
  dest=va+i+5+struct.unpack_from('<i',d,i+1)[0]
  if dest not in targets:continue
  out.append(f'CALL {va+i:08x} -> {dest:08x}')
  if dest==0x9afa36:continue
  lo=max(0,i-64)
  for ins in md.disasm(d[lo:i+180],va+lo):out.append(f'{ins.address:08x} {ins.mnemonic} {ins.op_str}')
(R/'reports/input-bootstrap-callers.txt').write_text('\n'.join(out));print('\n'.join(out))

