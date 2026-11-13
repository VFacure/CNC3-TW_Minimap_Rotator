"""Map MSVC x86 RTTI descriptors to candidate vtables, read-only."""
from pathlib import Path
import sys,struct,json,re
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools/python_deps'))
import pefile
p=pefile.PE(r'D:\SteamLibrary\steamapps\common\Command and Conquer 3 Tiberium Wars\RetailExe\1.9\cnc3game.dat');b=p.get_memory_mapped_image();base=p.OPTIONAL_HEADER.ImageBase
U=lambda o:struct.unpack_from('<I',b,o)[0]
def refs(v):
 needle=struct.pack('<I',v);i=0
 while (i:=b.find(needle,i))>=0:
  yield i;i+=1
out=[]
for m in re.finditer(rb'\.\?AV[^\x00]+\x00',b):
 name=m.group()[:-1].decode('ascii',errors='replace')
 if not any(x in name for x in ['Minimap','MiniMap','RenderingSurface']):continue
 td=base+m.start()-8;row={'type':name,'descriptor':hex(td),'vtables':[]}
 for ref in refs(td):
  col=ref-12
  if col<0 or U(col)!=0:continue
  for vref in refs(base+col):
   entries=[]
   for off in range(vref+4,min(vref+164,len(b)-4),4):
    fn=U(off)
    if not any(base+s.VirtualAddress<=fn<base+s.VirtualAddress+s.Misc_VirtualSize and s.Characteristics&0x20000000 for s in p.sections):break
    entries.append(hex(fn))
   if entries:row['vtables'].append({'address':hex(base+vref+4),'object_offset':U(col+4),'functions':entries})
 out.append(row)
(R/'reports/minimap-rtti.json').write_text(json.dumps(out,indent=2));print(json.dumps(out,indent=2))
