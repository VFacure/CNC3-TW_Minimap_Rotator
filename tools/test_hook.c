#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../source/hook.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int replacement(void){return 99;}
int main(void){
 BYTE body[]={0x55,0x8b,0xec,0x83,0xec,0x0c,0xb8,7,0,0,0,0xc9,0xc3};
 BYTE *p=VirtualAlloc(NULL,sizeof(body),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
 assert(p);memcpy(p,body,sizeof(body));DWORD old;assert(VirtualProtect(p,sizeof(body),PAGE_EXECUTE_READ,&old));
 assert(FlushInstructionCache(GetCurrentProcess(),p,sizeof(body)));
 int(*fn)(void)=(int(*)(void))p;MRHook h={0};assert(fn()==7);
 BYTE wrong[6]={0};assert(!mr_hook_install(&h,p,replacement,wrong));assert(fn()==7);
 assert(mr_hook_install(&h,p,replacement,body)==1);assert(fn()==99);
 assert(((int(*)(void))h.trampoline)()==7);
 assert(!mr_hook_install(&h,p,replacement,body));
 assert(mr_hook_remove(&h)==1);assert(fn()==7);assert(!memcmp(p,body,sizeof(body)));
 assert(!mr_hook_remove(&h));VirtualFree(p,0,MEM_RELEASE);
 puts("PASS: original, detour, trampoline, restoration, bad signature, duplicate install/remove. No game process used.");return 0;
}

