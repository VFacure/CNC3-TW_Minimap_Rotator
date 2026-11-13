"""Publish the hash-independent Minimap Rotator 1.0.2 maintenance release."""
from pathlib import Path
import sys,json,datetime
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'_GitHubPublishing'))
import github_publish as g
P=Path(__file__).resolve().parents[1];R=P/'repository';owner='VFacure';name='CNC3-TW_Minimap_Rotator';tag='v1.0.2'
s=g.session();remote=g.api(s,'GET',f'/repos/{owner}/{name}');assert remote['full_name']==owner+'/'+name
files=sorted(set(g.git(R,'diff','--name-only').splitlines())|set(g.git(R,'ls-files','--others','--exclude-standard').splitlines()))
for i in range(0,len(files),10):g.git(R,'add','--',*files[i:i+10])
if g.git(R,'status','--porcelain'):g.git(R,'commit','-m','Remove executable hash requirement and support 4GB-patched Tiberium Wars')
commit=g.git(R,'rev-parse','HEAD');branch=g.git(R,'branch','--show-current');g.git(R,'push','origin',branch)
if not g.git(R,'tag','--list',tag):g.git(R,'tag',tag,commit)
assert g.git(R,'rev-parse',tag)==commit
g.git(R,'push','origin',tag)
body=(P/'reports/release-notes-1.0.2.md').read_text()+'''\nInstall: close Tiberium Wars and replace `RetailExe/1.9/dinput8.dll` with the DLL from the ZIP. Hold Alt while rotating the camera to use the minimap rotation. Release-file checksums remain available for download verification; they are no longer an executable activation requirement.\n'''
releases=g.api(s,'GET',f'/repos/{owner}/{name}/releases?per_page=100');release=next((r for r in releases if r['tag_name']==tag),None)
if release is None:release=g.api(s,'POST',f'/repos/{owner}/{name}/releases',json={'tag_name':tag,'target_commitish':commit,'name':'Minimap Rotator 1.0.2','body':body,'draft':True,'prerelease':False})
paths=[R/'dist/Minimap_Rotator-1.0.2.zip',R/'dist/SHA256SUMS.txt']
assets=[g.upload_asset(owner,name,release['id'],p) for p in paths]
release=g.api(s,'PATCH',f'/repos/{owner}/{name}/releases/{release["id"]}',json={'draft':False,'make_latest':'true','body':body})
for a,p in zip(assets,paths):assert a['digest']=='sha256:'+g.sha(p) and a['size']==p.stat().st_size
assert g.api(s,'GET',f'/repos/{owner}/{name}/releases/latest')['tag_name']==tag
record={'repository':remote['html_url'],'release':release['html_url'],'tag':tag,'commit':commit,'assets':[{'name':a['name'],'size':a['size'],'digest':a['digest']} for a in assets],'published':not release['draft'],'private':remote['private'],'tag_matches_source_commit':True,'asset_digests_match_local_package':True,'verified_at':datetime.datetime.now().astimezone().isoformat()}
(P/'reports/github-release-1.0.2.json').write_text(json.dumps(record,indent=2))
for p in [P/'release/release-manifest.json',P/'reports/deployment-1.0.2.json']:
 data=json.loads(p.read_text());data.update(commit=commit,github_published=True);p.write_text(json.dumps(data,indent=2))
print(json.dumps(record,indent=2))
