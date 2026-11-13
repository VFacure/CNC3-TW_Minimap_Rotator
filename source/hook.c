#include "hook.h"
#include <stdint.h>
#include <string.h>
_Static_assert(sizeof(void*)==4,"x86 hook required");
static void jump(BYTE *out,const void *from,const void *to){
 uint32_t relative=(uint32_t)(uintptr_t)to-(uint32_t)(uintptr_t)from-5;
 out[0]=0xe9;memcpy(out+1,&relative,4);
}
int mr_hook_install(MRHook *h,void *target,void *replacement,const BYTE expected[6]){
 const BYTE safe[6]={0x55,0x8b,0xec,0x83,0xec,0x0c};
 MEMORY_BASIC_INFORMATION m;DWORD old,temp;
 if(!h||h->active||!target||!replacement||!expected||memcmp(expected,safe,6))return 0;
 if(VirtualQuery(target,&m,sizeof(m))!=sizeof(m)||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return 0;
 if((uintptr_t)target+6>(uintptr_t)m.BaseAddress+m.RegionSize||memcmp(target,expected,6))return 0;
 BYTE *tramp=VirtualAlloc(NULL,11,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!tramp)return 0;
 memcpy(tramp,expected,6);jump(tramp+6,tramp+6,(BYTE*)target+6);
 if(!VirtualProtect(tramp,11,PAGE_EXECUTE_READ,&temp)){VirtualFree(tramp,0,MEM_RELEASE);return 0;}
 if(!FlushInstructionCache(GetCurrentProcess(),tramp,11)){VirtualFree(tramp,0,MEM_RELEASE);return 0;}
 if(!VirtualProtect(target,6,PAGE_EXECUTE_READWRITE,&old)){VirtualFree(tramp,0,MEM_RELEASE);return 0;}
 if(memcmp(target,expected,6)){VirtualProtect(target,6,old,&temp);VirtualFree(tramp,0,MEM_RELEASE);return 0;}
 h->target=target;h->trampoline=tramp;memcpy(h->original,expected,6);
 jump(h->patch,target,replacement);h->patch[5]=0x90;
 memcpy(target,h->patch,6);h->active=1;
 int ok=FlushInstructionCache(GetCurrentProcess(),target,6)!=0;
 ok=VirtualProtect(target,6,old,&temp)!=0&&ok;
 /* -1 means installed but protection/cache operation failed: never unload. */
 return ok?1:-1;
}
int mr_hook_remove(MRHook *h){
 DWORD old,temp;if(!h||!h->active)return 0;
 if(memcmp(h->target,h->patch,6))return 0; /* Another patch owns these bytes. */
 if(!VirtualProtect(h->target,6,PAGE_EXECUTE_READWRITE,&old))return 0;
 memcpy(h->target,h->original,6);
 int ok=FlushInstructionCache(GetCurrentProcess(),h->target,6)!=0;
 ok=VirtualProtect(h->target,6,old,&temp)!=0&&ok;
 if(!ok)return -1;
 h->active=0;VirtualFree(h->trampoline,0,MEM_RELEASE);h->trampoline=NULL;return 1;
}
