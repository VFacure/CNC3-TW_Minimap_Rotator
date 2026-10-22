#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
typedef struct {unsigned level;const char *path;} AptRef;
typedef double (__cdecl *GetNumber)(const AptRef*,const char*);
typedef void (__cdecl *SetVariable)(const char*,const char*);
void mr_radar_layout(void*);
void mr_test_layout_api(GetNumber,SetVariable);
static double x=5,y=35,w=170,h=110,bw=170;static unsigned writes,layouts;static double lx,ly,lw,lh;
void mr_composite_layout(double a,double b,double c,double d){lx=a;ly=b;lw=c;lh=d;layouts++;}
void mr_log(const char *fmt,...){(void)fmt;}
static double __cdecl get(const AptRef *r,const char *key){
 if(strstr(r->path,"inputCatcher")){
  if(!strcmp(key,"_x"))return x;if(!strcmp(key,"_y"))return y;
  if(!strcmp(key,"_width"))return w;if(!strcmp(key,"_height"))return h;
 }else{
  if(!strcmp(key,"mapX")||!strcmp(key,"backX")||!strcmp(key,"backY"))return 5;
  if(!strcmp(key,"mapY"))return 35;if(!strcmp(key,"mapWidth"))return 170;if(!strcmp(key,"mapHeight"))return 110;
  if(!strcmp(key,"backWidth"))return bw;if(!strcmp(key,"backHeight"))return 170;
 }return 0;
}
static void __cdecl put(const char *path,const char *value){
 writes++;double v=atof(value);
 if(strstr(path,"._x"))x=v;else if(strstr(path,"._y"))y=v;
 else if(strstr(path,"._width"))w=v;else if(strstr(path,"._height"))h=v;
}
#define CHECK(v) do{if(!(v)){printf("FAIL %d: %s\n",__LINE__,#v);return 1;}}while(0)
int main(void){
 BYTE gadget[32]={0},radar[32]={0};*(uintptr_t*)gadget=0xaff914;
 AptRef *r=(AptRef*)(gadget+4);r->level=2;r->path="hud.radar";
 *(BYTE**)(radar+4)=gadget;mr_test_layout_api(get,put);mr_radar_layout(radar);
 CHECK(layouts==1&&writes==2);CHECK(x==5&&y==5&&w==170&&h==170);
 CHECK(fabs(lx)<1e-9&&fabs(ly+30.0/110)<1e-9&&fabs(lw-1)<1e-9&&fabs(lh-170.0/110)<1e-9);
 mr_radar_layout(radar);CHECK(writes==2&&layouts==2);
 bw=100;mr_radar_layout(radar);CHECK(writes==2&&layouts==2);
 puts("PASS: full-window rectangle and expanded input catcher, including a valid zero flag byte; stable layout avoids repeated writes; invalid layout refused. No game used.");return 0;
}
