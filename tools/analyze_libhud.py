from pathlib import Path
import xml.etree.ElementTree as ET,json,hashlib
r=Path(__file__).resolve().parents[1];p=r/'extracted/Apt/data/aptui/libHUD.xml';t=ET.parse(p)
s=next(s for s in t.getroot().findall('sprite') if s.get('id')=='16')
report={'xml_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'export':'SimpleRenderingSurface','character':16,'frames':len(s.findall('./frames/frame')),'functions':[x.attrib for x in s.iter() if x.tag.startswith('definefunction')],'string_operations':[x.attrib for x in s.iter('string')],'finding':'The shipped surface sprite registers its placeholder with a native :Render handler; it contains no camera-yaw getter or orientation calculation. Native property behavior remains unverified.'}
(r/'reports/libhud-surface-analysis.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
