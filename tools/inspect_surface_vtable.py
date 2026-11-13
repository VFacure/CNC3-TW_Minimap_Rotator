from pathlib import Path
import sys,struct
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools/python_deps'))
import pefile
p=pefile.PE(r'D:\SteamLibrary\steamapps\common\Command and Conquer 3 Tiberium Wars\RetailExe\1.9\cnc3game.dat');b=p.get_memory_mapped_image();base=p.OPTIONAL_HEADER.ImageBase
out=[]
for dest in [0x9c14bf,0x9c14d2,0x9c145a,0xaff8ac]:
 start=0;needle=struct.pack('<I',dest)
 while (i:=b.find(needle,start))>=0:
  start=i+1;out.append(f'{base+i:08x} -> {dest:08x}')
  if dest==0x9c14bf:
   for off in range(i-32,i+64,4):out.append(f'  {base+off:08x}: {struct.unpack_from("<I",b,off)[0]:08x}')
(R/'reports/surface-vtable-references.txt').write_text('\n'.join(out));print('\n'.join(out))
