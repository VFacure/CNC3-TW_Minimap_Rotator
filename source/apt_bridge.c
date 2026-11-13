#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "camera_adapter.h"
#include "minimap_transform.h"
#include "apt_bridge.h"
static int ready;
typedef void (__cdecl *SetVariable)(const char*,const char*);
int mr_apt_bind(void){
 static const unsigned char signature[]={0x55,0x8b,0xec,0x83,0xec,0x0c,0x33,0xc0,0x40};
 ready=mr_camera_bind() && !memcmp((void*)0x852829,signature,sizeof(signature));return ready;
}
static int set(unsigned level,const char *root,const char *key,double number){
 char path[1024],value[64];
 int n=snprintf(path,sizeof(path),"_level%u.%s.panel.%s",level,root,key);
 if(n<0||(size_t)n>=sizeof(path))return 0;
 n=snprintf(value,sizeof(value),"%.9g",number);
 if(n<0||(size_t)n>=sizeof(value))return 0;
 ((SetVariable)0x852829)(path,value);return 1;
}
int mr_apt_panel_rect(unsigned level,const char *root,double map_x,double map_y,double w,double h,double angle){
 MRTransform t;
 if(!ready||!root||!*root||strlen(root)>900||level>13||!mr_transform(&t,w,h,angle))return 0;
 /* Assumes the stock panel's untransformed origin is (0,0).
    A host HUD with a different origin must provide an adapted bridge. */
 t.cx+=map_x; t.cy+=map_y;
 double x=t.cx-t.scale*(t.c*t.cx-t.s*t.cy);
 double y=t.cy-t.scale*(t.s*t.cx+t.c*t.cy);
 return set(level,root,"_xscale",100*t.scale)&&set(level,root,"_yscale",100*t.scale)
  &&set(level,root,"_rotation",angle*180/acos(-1))&&set(level,root,"_x",x)&&set(level,root,"_y",y);
}

int mr_apt_panel(unsigned level,const char *root,double w,double h,double angle){return mr_apt_panel_rect(level,root,0,0,w,h,angle);}
