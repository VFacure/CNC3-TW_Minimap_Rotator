#include <windows.h>
#include <wchar.h>
#include "executable_guard.h"
/* Identify the engine without a hash, timestamp or checksum allowlist.
   camera_adapter/runtime separately validate the actual hook layout. */
int mr_executable_identity_supported(const WCHAR *path,const BYTE *headers,SIZE_T available){
 if(!path||!headers||available<sizeof(IMAGE_DOS_HEADER))return 0;
 const WCHAR *name=path;
 for(const WCHAR *p=path;*p;p++)if(*p==L'\\'||*p==L'/')name=p+1;
 if(_wcsicmp(name,L"cnc3game.dat")&&_wcsicmp(name,L"cnc3game.exe"))return 0;
 const IMAGE_DOS_HEADER *dos=(const void*)headers;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<(LONG)sizeof(*dos))return 0;
 SIZE_T pe=(SIZE_T)dos->e_lfanew;
 if(pe>available||available-pe<sizeof(IMAGE_NT_HEADERS32))return 0;
 const IMAGE_NT_HEADERS32 *nt=(const void*)(headers+pe);
 return nt->Signature==IMAGE_NT_SIGNATURE && nt->FileHeader.Machine==IMAGE_FILE_MACHINE_I386 &&
        !(nt->FileHeader.Characteristics&IMAGE_FILE_DLL) &&
        nt->FileHeader.SizeOfOptionalHeader>=sizeof(IMAGE_OPTIONAL_HEADER32) &&
        nt->OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR32_MAGIC;
}
int mr_executable_verified(void){
 WCHAR path[32768];DWORD n=GetModuleFileNameW(NULL,path,32768);
 const BYTE *base=(const BYTE*)GetModuleHandleW(NULL);MEMORY_BASIC_INFORMATION region;
 if(!n||n>=32768||!base||VirtualQuery(base,&region,sizeof(region))!=sizeof(region))return 0;
 if(region.State!=MEM_COMMIT||(region.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return 0;
 SIZE_T available=region.RegionSize-((SIZE_T)base-(SIZE_T)region.BaseAddress);
 return mr_executable_identity_supported(path,base,available);
}
