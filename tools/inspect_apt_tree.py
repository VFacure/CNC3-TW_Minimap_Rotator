"""Inspect the APT display tree without rewriting original assets."""
from pathlib import Path
import struct,json
ROOT=Path(__file__).resolve().parents[1]
p=ROOT/'extracted/Apt/data/aptui'
b=(p/'TacticalHUDRadar.apt').read_bytes(); c=(p/'TacticalHUDRadar.const').read_bytes()
def u(o):
 assert 0<=o<=len(b)-4,o
 return struct.unpack_from('<I',b,o)[0]
def s(o):
 if not o:return None
 assert 0<o<len(b),o
 return b[o:b.index(0,o)].decode('utf-8',errors='replace')
m=struct.unpack_from('<I',c,20)[0]
count,table=u(m+20),u(m+24)
result=[]
for idx in range(count):
 off=u(table+idx*4)
 if not off:continue
 typ=u(off)
 if typ not in (5,9):continue
 frames=[]
 for fi in range(u(off+8)):
  f=u(off+12)+fi*8; items=[]
  for j in range(u(f)):
   q=u(u(f+4)+j*4); t=u(q); d={'type':t,'offset':q}
   if t==3:
    d.update(flags=u(q+4),depth=u(q+8),character=u(q+12),matrix=struct.unpack_from('<6f',b,q+16),name=s(u(q+52)),clipdepth=u(q+56),actions=u(q+60))
   elif t==1:d['bytecode']=u(q+4)
   elif t==2:d['label']=s(u(q+4))
   items.append(d)
  frames.append({'frame':fi,'items':items})
 result.append({'character':idx,'offset':off,'type':typ,'frames':frames})
(ROOT/'reports/apt-display-tree.json').write_text(json.dumps(result,indent=2))
for ch in result:
 for fr in ch['frames']:
  for d in fr['items']:
   if d.get('name'):print(ch['character'],fr['frame'],d['name'],d['character'],d['matrix'])
print('Parsed',len(result),'movie/sprite definitions; all offsets bounds checked.')

