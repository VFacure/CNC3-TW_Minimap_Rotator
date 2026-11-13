/* Update integration, not yet installed as a detour. */
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include "camera_adapter.h"
#include "apt_bridge.h"
typedef struct { unsigned level;const char *path; } AptRef;
typedef double (__cdecl *GetNumber)(const AptRef*,const char*);
int mr_radar_tick(void *radar){
 if(!radar)return 0;
 unsigned char *gadget=*(unsigned char**)((unsigned char*)radar+4);
 if(!gadget||*(uintptr_t*)gadget!=0xaff914||!gadget[0x0c])return 0;
 const AptRef *ref=(const AptRef*)(gadget+4);
 if(ref->level>13||!ref->path)return 0;
 GetNumber get=(GetNumber)0x9a6105;
 double w=get(ref,"mapWidth"),h=get(ref,"mapHeight");
 double x=get(ref,"mapX"),y=get(ref,"mapY"),a;
 if(!isfinite(x)||!isfinite(y)||!isfinite(w)||!isfinite(h)||w<=0||h<=0)return 0;
 if(!mr_camera_angle(&a))return 0;
 return mr_apt_panel_rect(ref->level,ref->path,x,y,w,h,a);
}
