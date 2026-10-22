#include <windows.h>
#include <unknwn.h>
#include <stdio.h>
typedef HRESULT(WINAPI *Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
typedef HRESULT(WINAPI *CanUnload)(void);
int main(int argc,char **argv){
 if(argc!=2)return 10;
 HMODULE proxy=LoadLibraryA(argv[1]);if(!proxy){printf("Load error %lu\n",GetLastError());return 11;}
 WCHAR path[MAX_PATH];UINT n=GetSystemDirectoryW(path,MAX_PATH);if(!n||n>=MAX_PATH-13)return 12;
 memcpy(path+n,L"\\dinput8.dll",13*sizeof(WCHAR));
 HMODULE real=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!real)return 13;
 const char *names[]={"DirectInput8Create","DllCanUnloadNow","DllGetClassObject","DllRegisterServer","DllUnregisterServer","GetdfDIJoystick"};
 for(unsigned i=0;i<6;i++)if(!GetProcAddress(proxy,names[i])||!GetProcAddress(proxy,MAKEINTRESOURCEA(i+1)))return 14;
 Create a=(Create)(void*)GetProcAddress(proxy,names[0]),b=(Create)(void*)GetProcAddress(real,names[0]);
 const IID iid={0xbf798030,0x483a,0x4da2,{0xaa,0x99,0x5d,0x64,0xed,0x36,0x97,0x00}};
 void *x=NULL,*y=NULL;
 HRESULT hr1=a(GetModuleHandleW(NULL),0x0800,&iid,&x,NULL),hr2=b(GetModuleHandleW(NULL),0x0800,&iid,&y,NULL);
 printf("Proxy HRESULT=%08lx; system HRESULT=%08lx\n",(unsigned long)hr1,(unsigned long)hr2);
 if(hr1!=hr2)return 15;
 if(SUCCEEDED(hr1)&&(!x||!y))return 16;
 if(x)((IUnknown*)x)->lpVtbl->Release((IUnknown*)x);
 if(y)((IUnknown*)y)->lpVtbl->Release((IUnknown*)y);
 typedef DWORD(WINAPI *Status)(void*);Status status=(Status)(void*)GetProcAddress(proxy,"MR_Status@4");
 if(!status||status(NULL)!=0)return 17;
 puts("PASS: all six exports by name and ordinal; real DirectInput interface result preserved; game hooks inactive. No game launched.");
 FreeLibrary(proxy);FreeLibrary(real);return 0;
}
