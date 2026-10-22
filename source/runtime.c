#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <d3dx9.h>
#include "composite.h"
#include "../tools/minhook/include/MinHook.h"
void mr_radar_layout(void*);
typedef void (__thiscall *RadarUpdate)(void*);
static RadarUpdate radar_original;
static void __fastcall radar_update(void *self,void *unused){(void)unused;radar_original(self);mr_radar_layout(self);}
typedef struct {float x,y;} Vec2;
typedef void (__thiscall *ViewBoxFn)(void*,const Vec2*,const Vec2*);
static ViewBoxFn viewbox_original=(ViewBoxFn)0x9c768b;
static void __fastcall viewbox(void *self,void *unused,const Vec2 *pos,const Vec2 *size){
 (void)unused;viewbox_original(self,pos,size);if(pos&&size)mr_composite_observe(pos->x,pos->y,size->x,size->y);
}
__attribute__((naked)) static void parse_coordinates(void){
 __asm__ volatile("pushl 4(%esp)\n\tmovl $0x9c1879,%eax\n\tcall *%eax\n\taddl $4,%esp\n\tpushl %eax\n\tpushl %esi\n\tcall _mr_input_remap\n\taddl $4,%esp\n\tpopl %eax\n\tret\n\t");
}
static HRESULT (WINAPI *real_texture)(LPDIRECT3DDEVICE9,UINT,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,LPDIRECT3DTEXTURE9*);
static HRESULT (WINAPI *real_effect)(LPDIRECT3DDEVICE9,LPCVOID,UINT,const D3DXMACRO*,LPD3DXINCLUDE,DWORD,LPD3DXEFFECTPOOL,LPD3DXEFFECT*,LPD3DXBUFFER*);
static HRESULT (WINAPI *real_file_effect)(LPDIRECT3DDEVICE9,LPCSTR,const D3DXMACRO*,LPD3DXINCLUDE,DWORD,LPD3DXEFFECTPOOL,LPD3DXEFFECT*,LPD3DXBUFFER*);
static HRESULT WINAPI texture(LPDIRECT3DDEVICE9 d,UINT w,UINT h,UINT m,DWORD u,D3DFORMAT f,D3DPOOL p,LPDIRECT3DTEXTURE9 *t){mr_composite_attach(d);return real_texture(d,w,h,m,u,f,p,t);}
static HRESULT WINAPI effect(LPDIRECT3DDEVICE9 d,LPCVOID s,UINT n,const D3DXMACRO *m,LPD3DXINCLUDE i,DWORD f,LPD3DXEFFECTPOOL p,LPD3DXEFFECT *e,LPD3DXBUFFER *b){mr_composite_attach(d);return real_effect(d,s,n,m,i,f,p,e,b);}
static HRESULT WINAPI file_effect(LPDIRECT3DDEVICE9 d,LPCSTR s,const D3DXMACRO *m,LPD3DXINCLUDE i,DWORD f,LPD3DXEFFECTPOOL p,LPD3DXEFFECT *e,LPD3DXBUFFER *b){mr_composite_attach(d);return real_file_effect(d,s,m,i,f,p,e,b);}
typedef struct {BYTE *at;BYTE original[5];SIZE_T size;} Patch;
static Patch patches[16];static unsigned patch_count;
static int write_patch(void *at,const void *data,SIZE_T size){
 if(patch_count>=16||size>5)return 0;DWORD old,temp;
 if(!VirtualProtect(at,size,PAGE_EXECUTE_READWRITE,&old))return 0;
 Patch *p=&patches[patch_count++];p->at=at;p->size=size;memcpy(p->original,at,size);memcpy(at,data,size);
 int ok=FlushInstructionCache(GetCurrentProcess(),at,size)!=0;
 if(!VirtualProtect(at,size,old,&temp))ok=0;return ok;
}
static void rollback(void){while(patch_count){Patch *p=&patches[--patch_count];DWORD old,temp;if(VirtualProtect(p->at,p->size,PAGE_EXECUTE_READWRITE,&old)){memcpy(p->at,p->original,p->size);FlushInstructionCache(GetCurrentProcess(),p->at,p->size);VirtualProtect(p->at,p->size,old,&temp);}}}
int mr_runtime_install(void){
 const BYTE update_prefix[6]={0x55,0x8b,0xec,0x83,0xec,0x0c};
 if(memcmp((void*)0x9b1eb1,update_prefix,6))return 0;
 const uintptr_t calls[]={0x9c19fb,0x9c1a75,0x9c1aff,0x9c1b25,0x9c1baf,0x9c1bd5};
 if(*(uintptr_t*)0xb005c8!=0x9c768b)return 0;
 for(unsigned i=0;i<6;i++){BYTE *at=(BYTE*)calls[i];int32_t rel;memcpy(&rel,at+1,4);if(at[0]!=0xe8||calls[i]+5+rel!=0x9c1879)return 0;}
 BYTE *base=(BYTE*)GetModuleHandleW(NULL);IMAGE_DOS_HEADER *dos=(void*)base;IMAGE_NT_HEADERS32 *nt=(void*)(base+dos->e_lfanew);
 IMAGE_IMPORT_DESCRIPTOR *imports=(void*)(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
 MH_STATUS mh=MH_Initialize();if(mh!=MH_OK&&mh!=MH_ERROR_ALREADY_INITIALIZED)return 0;
 if(MH_CreateHook((void*)0x9b1eb1,(void*)&radar_update,(void**)&radar_original)!=MH_OK)return 0;
 unsigned found=0;
 for(;imports->Name;imports++){
  if(_stricmp((char*)base+imports->Name,"d3dx9_29.dll"))continue;
  IMAGE_THUNK_DATA32 *name=(void*)(base+imports->OriginalFirstThunk),*slot=(void*)(base+imports->FirstThunk);
  for(;name->u1.AddressOfData;name++,slot++){
   if(IMAGE_SNAP_BY_ORDINAL32(name->u1.Ordinal))continue;
   IMAGE_IMPORT_BY_NAME *info=(void*)(base+name->u1.AddressOfData);void *replacement=NULL;
   if(!strcmp((char*)info->Name,"D3DXCreateTexture")){memcpy(&real_texture,&slot->u1.Function,4);replacement=(void*)&texture;}
   if(!strcmp((char*)info->Name,"D3DXCreateEffect")){memcpy(&real_effect,&slot->u1.Function,4);replacement=(void*)&effect;}
   if(!strcmp((char*)info->Name,"D3DXCreateEffectFromFileA")){memcpy(&real_file_effect,&slot->u1.Function,4);replacement=(void*)&file_effect;}
   if(replacement){if(!write_patch(&slot->u1.Function,&replacement,4))goto fail;found++;}
  }
 }
 if(!found)goto fail;
 void *render=(void*)&viewbox;if(!write_patch((void*)0xb005c8,&render,4))goto fail;
 for(unsigned i=0;i<6;i++){BYTE patch[5]={0xe8};uint32_t rel=(uint32_t)(uintptr_t)&parse_coordinates-(uint32_t)calls[i]-5;memcpy(patch+1,&rel,4);if(!write_patch((void*)calls[i],patch,5))goto fail;}
 if(MH_EnableHook((void*)0x9b1eb1)!=MH_OK)goto fail;
 mr_log("Runtime installed: %u D3DX capture imports, view-box observer, six coordinate parsers",found);return 1;
 fail:MH_DisableHook((void*)0x9b1eb1);MH_RemoveHook((void*)0x9b1eb1);rollback();mr_log("Runtime installation rolled back");return 0;
}

#ifdef MR_TESTING
void *mr_test_parser(void){return (void*)&parse_coordinates;}
#endif
