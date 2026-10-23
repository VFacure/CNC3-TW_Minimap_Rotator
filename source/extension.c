#include <windows.h>
#include "camera_adapter.h"
#include "composite.h"
#include "executable_guard.h"
int mr_runtime_install(void);
static LONG state;
__declspec(dllexport) DWORD WINAPI MR_Initialize(void *reserved){
 (void)reserved;
 if(InterlockedCompareExchange(&state,1,0)!=0)return 3;
 if(!mr_executable_verified()||!mr_camera_bind()){state=0;return 1;}
 HMODULE pinned;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)(void*)&MR_Initialize,&pinned)){state=0;return 2;}
 mr_log("Minimap Rotator 1.0.2 initializing (PID %lu)",GetCurrentProcessId());
 if(!mr_runtime_install()){state=0;return 2;}
 state=2;return 0;
}
__declspec(dllexport) DWORD WINAPI MR_Status(void *reserved){(void)reserved;return (DWORD)state;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)DisableThreadLibraryCalls(h);return TRUE;}
