/* Project-local DirectInput proxy. All real input behavior is forwarded to the
   Windows copy loaded by absolute system path. No loader work in DllMain. */
#include <windows.h>
#include <unknwn.h>
#include <dinput.h>
#ifndef MR_ENABLE_GAME_HOOKS
#define MR_ENABLE_GAME_HOOKS 0
#endif
DWORD WINAPI MR_Initialize(void*);
static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
static HMODULE original;
static FARPROC procedures[6];
static BOOL CALLBACK load_original(PINIT_ONCE o,PVOID p,PVOID *context){
 (void)o;(void)p;(void)context;
 WCHAR path[MAX_PATH];UINT n=GetSystemDirectoryW(path,MAX_PATH);
 if(!n||n>=MAX_PATH-13)return FALSE;
 memcpy(path+n,L"\\dinput8.dll",13*sizeof(WCHAR));
 original=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);
 if(!original)return FALSE;
 const char *names[]={"DirectInput8Create","DllCanUnloadNow","DllGetClassObject","DllRegisterServer","DllUnregisterServer","GetdfDIJoystick"};
 for(unsigned i=0;i<6;i++)procedures[i]=GetProcAddress(original,names[i]);
 return procedures[0]!=NULL;
}
static FARPROC resolve(unsigned index){
 if(!InitOnceExecuteOnce(&once,load_original,NULL,NULL))return NULL;
 return procedures[index];
}
HRESULT WINAPI DirectInput8Create(HINSTANCE instance,DWORD version,REFIID iid,LPVOID *out,LPUNKNOWN outer){
 typedef HRESULT(WINAPI *Fn)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
 Fn fn=(Fn)(void*)resolve(0);if(!fn){if(out)*out=NULL;return E_FAIL;}
 #if MR_ENABLE_GAME_HOOKS
 /* The initializer verifies the host; overlays can change the input caller. */
 MR_Initialize(NULL);
 #endif
 return fn(instance,version,iid,out,outer);
}
HRESULT WINAPI DllCanUnloadNow(void){
 typedef HRESULT(WINAPI *Fn)(void);Fn fn=(Fn)(void*)resolve(1);
 return fn?fn():S_FALSE;
}
HRESULT WINAPI DllGetClassObject(REFCLSID clsid,REFIID iid,LPVOID *out){
 typedef HRESULT(WINAPI *Fn)(REFCLSID,REFIID,LPVOID*);Fn fn=(Fn)(void*)resolve(2);
 if(!fn){if(out)*out=NULL;return CLASS_E_CLASSNOTAVAILABLE;}return fn(clsid,iid,out);
}
HRESULT WINAPI DllRegisterServer(void){typedef HRESULT(WINAPI *Fn)(void);Fn fn=(Fn)(void*)resolve(3);return fn?fn():E_FAIL;}
HRESULT WINAPI DllUnregisterServer(void){typedef HRESULT(WINAPI *Fn)(void);Fn fn=(Fn)(void*)resolve(4);return fn?fn():E_FAIL;}
LPCDIDATAFORMAT WINAPI GetdfDIJoystick(void){typedef LPCDIDATAFORMAT(WINAPI *Fn)(void);Fn fn=(Fn)(void*)resolve(5);return fn?fn():NULL;}
