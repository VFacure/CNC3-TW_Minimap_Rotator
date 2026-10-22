#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "composite.h"
typedef struct {unsigned level;const char *path;} AptRef;
typedef double (__cdecl *GetNumber)(const AptRef*,const char*);
typedef void (__cdecl *SetVariable)(const char*,const char*);
static GetNumber read_number=(GetNumber)0x9a6105;
static SetVariable write_variable=(SetVariable)0x852829;
#ifdef MR_TESTING
void mr_test_layout_api(GetNumber get,SetVariable put){read_number=get;write_variable=put;}
#endif
static void set(const AptRef *r,const char *key,double v){
 char path[1024],value[64];int n=snprintf(path,sizeof(path),"_level%u.%s.panel.inputCatcher.%s",r->level,r->path,key);
 if(n<0||(size_t)n>=sizeof(path))return;snprintf(value,sizeof(value),"%.9g",v);write_variable(path,value);
}
void mr_radar_layout(void *radar){
 BYTE *g=*(BYTE**)((BYTE*)radar+4);if(!g||*(uintptr_t*)g!=0xaff914)return;
 const AptRef *r=(const AptRef*)(g+4);if(r->level>13||!r->path||strlen(r->path)>850)return;
 GetNumber get=read_number;
 double mx=get(r,"mapX"),my=get(r,"mapY"),mw=get(r,"mapWidth"),mh=get(r,"mapHeight");
 double bx=get(r,"backX"),by=get(r,"backY"),bw=get(r,"backWidth"),bh=get(r,"backHeight");
 if(!isfinite(mx)||!isfinite(my)||!isfinite(mw)||!isfinite(mh)||!isfinite(bx)||!isfinite(by)||!isfinite(bw)||!isfinite(bh))return;
 if(mw<1||mh<1||bw<mw-1||bh<mh-1||bw>4096||bh>4096)return;
 mr_composite_layout((bx-mx)/mw,(by-my)/mh,bw/mw,bh/mh);
 char nested[1024];int n=snprintf(nested,sizeof(nested),"%s.panel.inputCatcher",r->path);if(n<0||(size_t)n>=sizeof(nested))return;
 AptRef catcher={r->level,nested};
 if(fabs(get(&catcher,"_x")-bx)>0.01)set(r,"_x",bx);
 if(fabs(get(&catcher,"_y")-by)>0.01)set(r,"_y",by);
 if(fabs(get(&catcher,"_width")-bw)>0.01)set(r,"_width",bw);
 if(fabs(get(&catcher,"_height")-bh)>0.01)set(r,"_height",bh);
 static int logged;if(!logged){mr_log("Minimap window layout: map=(%.3f,%.3f %.3fx%.3f), back=(%.3f,%.3f %.3fx%.3f)",mx,my,mw,mh,bx,by,bw,bh);logged=1;}
}
