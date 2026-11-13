from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools/python_deps'))
import pefile
from capstone import *
p=Path(r'D:\SteamLibrary\steamapps\common\Command and Conquer 3 Tiberium Wars\RetailExe\1.9\cnc3game.dat');pe=pefile.PE(str(p));im=pe.get_memory_mapped_image();base=pe.OPTIONAL_HEADER.ImageBase
md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
out=[]
for ins in md.disasm(im[0x4a5f01-base:0x4a5f8f-base],0x4a5f01):
 annotation=[]
 for op in ins.operands:
  if op.type==CS_OP_IMM and base<=op.imm<base+len(im):
   r=op.imm-base;raw=im[r:r+100].split(b'\0')[0]
   if len(raw)>3 and all(32<=x<127 for x in raw):annotation.append(raw.decode())
 out.append(f'{ins.address:08x} {ins.mnemonic:8} {ins.op_str} '+repr(annotation) if annotation else f'{ins.address:08x} {ins.mnemonic:8} {ins.op_str}')
(ROOT/'reports/screen-to-world.txt').write_text('\n'.join(out))
print('\n'.join(x for x in out if '[' in x and "'" in x))

