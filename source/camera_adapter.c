#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "camera_adapter.h"
_Static_assert(sizeof(void*)==4,"Tiberium Wars adapter requires x86");
typedef struct {float x,y,z;} Point3;
typedef void (__cdecl *Corners)(Point3*,Point3*,Point3*,Point3*,float);
static int bound;
static int readable(const void *p,SIZE_T bytes){
 MEMORY_BASIC_INFORMATION m;
 if(!p||VirtualQuery(p,&m,sizeof(m))!=sizeof(m)||m.State!=MEM_COMMIT)return 0;
 if(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))return 0;
 uintptr_t a=(uintptr_t)p,e=(uintptr_t)m.BaseAddress+m.RegionSize;
 return a<e&&bytes<=e-a;
}
int mr_camera_bind(void){
 const BYTE *b=(const BYTE*)GetModuleHandleW(NULL);bound=0;
 if((uintptr_t)b!=0x400000||!readable(b,4096))return 0;
 const IMAGE_DOS_HEADER *dos=(const IMAGE_DOS_HEADER*)b;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>2048)return 0;
 const IMAGE_NT_HEADERS32 *nt=(const IMAGE_NT_HEADERS32*)(b+dos->e_lfanew);
 if(nt->Signature!=IMAGE_NT_SIGNATURE||nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||nt->OptionalHeader.SizeOfImage<0x8129e0)return 0;
 static const BYTE sig[]={0x55,0x8b,0xec,0x83,0xec,0x20,0x57,0x33,0xff,0x39,0x7d,0x08,0x0f,0x84,0x9f,0x00};
 if(!readable((void*)0x4a5f8f,sizeof(sig))||memcmp((void*)0x4a5f8f,sig,sizeof(sig)))return 0;
 bound=1;return 1;
}
int mr_camera_angle(double *angle){
 if(!bound||!angle)return 0;
 /* The projection helper dereferences the display and camera-manager globals. */
 void *display=*(void**)0xc090d0;BYTE *manager=*(BYTE**)0xc129dc;
 if(!readable(display,4)||!readable(manager,0x28))return 0;
 BYTE **start=*(BYTE***)(manager+0x18),**end=*(BYTE***)(manager+0x1c);
 unsigned index=*(unsigned*)(manager+0x24);
 uintptr_t lo=(uintptr_t)start,hi=(uintptr_t)end;
 if(hi<lo||hi-lo>4096||index>=(hi-lo)/4||!readable(start,(index+1)*4)||!readable(start[index],4))return 0;
 Point3 tl,tr,br,bl;
 ((Corners)0x4a5f8f)(&tl,&tr,&br,&bl,0.0f);
 double dx=(double)tr.x-tl.x,dy=(double)tr.y-tl.y;
 if(!isfinite(dx)||!isfinite(dy)||hypot(dx,dy)<1e-6)return 0;
 /* Map pixels invert world Y. Rotate the projected camera-right vector to +X. */
 *angle=atan2(dy,dx);return 1;
}
