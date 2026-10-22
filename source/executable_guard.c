#include <windows.h>
#include <bcrypt.h>
#include <string.h>
/* Full on-disk executable identity; complements instruction-level hook checks. */
int mr_executable_verified(void){
 static const BYTE expected[32]={0xfb,0x0c,0xb9,0xbb,0xe4,0x0a,0xd4,0x29,0x74,0x3d,0xda,0xbe,0x61,0x41,0x50,0x51,0x4f,0xff,0xa9,0xdd,0xbd,0xe1,0xe0,0x91,0xa7,0x76,0x0e,0x9a,0xb8,0xc2,0x19,0x58};
 WCHAR path[32768];DWORD n=GetModuleFileNameW(NULL,path,32768),read;BYTE hash[32],buffer[65536];
 BCRYPT_ALG_HANDLE alg=NULL;BCRYPT_HASH_HANDLE ctx=NULL;int ok=0;HANDLE file=INVALID_HANDLE_VALUE;
 if(!n||n>=32768)return 0;
 file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 if(file==INVALID_HANDLE_VALUE)goto done;
 if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,NULL,0)<0)goto done;
 if(BCryptCreateHash(alg,&ctx,NULL,0,NULL,0,0)<0)goto done;
 for(;;){if(!ReadFile(file,buffer,sizeof(buffer),&read,NULL))goto done;if(!read)break;if(BCryptHashData(ctx,buffer,read,0)<0)goto done;}
 if(BCryptFinishHash(ctx,hash,sizeof(hash),0)<0)goto done;
 ok=memcmp(hash,expected,sizeof(hash))==0;
 done:if(ctx)BCryptDestroyHash(ctx);if(alg)BCryptCloseAlgorithmProvider(alg,0);if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);return ok;
}
