#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
static SRWLOCK log_lock=SRWLOCK_INIT;
void mr_log(const char *format,...){
 WCHAR path[32768];HMODULE module=NULL;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)(void*)&mr_log,&module))return;
 DWORD n=GetModuleFileNameW(module,path,32768);if(!n||n>=32750)return;
 while(n&&path[n-1]!=L'\\')n--;if(!n)return;
 memcpy(path+n,L"MinimapRotator.log",19*sizeof(WCHAR));
 char line[2048];va_list args;va_start(args,format);int count=vsnprintf(line,sizeof(line)-3,format,args);va_end(args);
 if(count<0)return;size_t len=(size_t)count;if(len>sizeof(line)-3)len=sizeof(line)-3;
 line[len++]='\r';line[len++]='\n';
 AcquireSRWLockExclusive(&log_lock);
 HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
 if(file!=INVALID_HANDLE_VALUE){DWORD written;WriteFile(file,line,(DWORD)len,&written,NULL);CloseHandle(file);}
 ReleaseSRWLockExclusive(&log_lock);
}
