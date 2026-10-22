#include <windows.h>
#include <stdio.h>
typedef DWORD(WINAPI *Entry)(void*);
int main(int argc,char **argv){
 if(argc!=2)return 10;HMODULE m=LoadLibraryA(argv[1]);if(!m){printf("load error %lu\n",GetLastError());return 11;}
 Entry init=(Entry)(void*)GetProcAddress(m,"MR_Initialize@4");
 Entry status=(Entry)(void*)GetProcAddress(m,"MR_Status@4");
 if(!init)init=(Entry)(void*)GetProcAddress(m,"MR_Initialize");
 if(!status)status=(Entry)(void*)GetProcAddress(m,"MR_Status");
 if(!init||!status)return 12;
 if(status(NULL)!=0||init(NULL)!=1||status(NULL)!=0)return 13;
 if(init(NULL)!=1||status(NULL)!=0)return 14;
 if(!FreeLibrary(m))return 15;
 puts("PASS: DLL loads, exposes API, rejects unsupported executable twice, remains inactive, unloads. No game used.");return 0;
}
