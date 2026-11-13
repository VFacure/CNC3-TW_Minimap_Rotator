#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../source/composite.h"
void *mr_test_parser(void);
int mr_composite_attach(IDirect3DDevice9 *d){(void)d;return 1;}
void mr_composite_observe(float x,float y,float w,float h){(void)x;(void)y;(void)w;(void)h;}
__attribute__((used,noinline)) void mr_input_remap(float *p){p[0]+=2;p[1]+=3;}
void mr_log(const char *format,...){(void)format;}
__attribute__((naked)) static float *invoke(void *fn,const float *in,float *out){
 __asm__ volatile("pushl %esi\n\tmovl 16(%esp),%esi\n\tpushl 12(%esp)\n\tcall *12(%esp)\n\taddl $4,%esp\n\tpopl %esi\n\tret\n\t");
}
int main(void){
 BYTE *page=VirtualAlloc((void*)0x9c0000,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
 if(page!=(void*)0x9c0000)return 1;
 const BYTE parser[]={0x8b,0x44,0x24,0x04,0xf3,0x0f,0x10,0x00,0xf3,0x0f,0x11,0x06,0xf3,0x0f,0x10,0x40,0x04,0xf3,0x0f,0x11,0x46,0x04,0x89,0xf0,0xc3};
 memcpy(page+0x1879,parser,sizeof(parser));DWORD old;
 if(!VirtualProtect(page,8192,PAGE_EXECUTE_READ,&old)||!FlushInstructionCache(GetCurrentProcess(),page,8192))return 2;
 float in[2]={0.1f,0.2f},out[2]={0};
 float *result=invoke(mr_test_parser(),in,out);
 if(result!=out||fabs(out[0]-2.1f)>1e-6||fabs(out[1]-3.2f)>1e-6)return 3;
 VirtualFree(page,0,MEM_RELEASE);
 puts("PASS: native radar parser wrapper preserves its ESI output buffer, argument stack and return pointer, then applies remapping. Separate test process only.");return 0;
}


