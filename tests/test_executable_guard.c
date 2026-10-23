#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../source/executable_guard.h"
#define CHECK(c) do{if(!(c)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void){
 BYTE image[4096]={0};IMAGE_DOS_HEADER *dos=(void*)image;dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=128;
 IMAGE_NT_HEADERS32 *nt=(void*)(image+128);nt->Signature=IMAGE_NT_SIGNATURE;nt->FileHeader.Machine=IMAGE_FILE_MACHINE_I386;
 nt->FileHeader.SizeOfOptionalHeader=sizeof(IMAGE_OPTIONAL_HEADER32);nt->OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR32_MAGIC;
 nt->FileHeader.Characteristics=IMAGE_FILE_EXECUTABLE_IMAGE|IMAGE_FILE_32BIT_MACHINE;
 CHECK(mr_executable_identity_supported(L"D:\\Games\\cnc3game.dat",image,sizeof(image)));
 nt->FileHeader.Characteristics|=IMAGE_FILE_LARGE_ADDRESS_AWARE;
 CHECK(mr_executable_identity_supported(L"D:\\Games\\CNC3GAME.DAT",image,sizeof(image)));
 nt->FileHeader.TimeDateStamp=0x12345678;nt->OptionalHeader.CheckSum=0xabcdef;image[4000]=0x55;
 CHECK(mr_executable_identity_supported(L"D:/Games/cnc3game.exe",image,sizeof(image)));
 CHECK(!mr_executable_identity_supported(L"worldbuilder.exe",image,sizeof(image)));
 CHECK(!mr_executable_identity_supported(L"ra3game.dat",image,sizeof(image)));
 CHECK(!mr_executable_identity_supported(NULL,image,sizeof(image)));
 CHECK(!mr_executable_identity_supported(L"cnc3game.dat",NULL,sizeof(image)));
 CHECK(!mr_executable_identity_supported(L"cnc3game.dat",image,128));
 nt->FileHeader.Machine=IMAGE_FILE_MACHINE_AMD64;CHECK(!mr_executable_identity_supported(L"cnc3game.dat",image,sizeof(image)));nt->FileHeader.Machine=IMAGE_FILE_MACHINE_I386;
 nt->FileHeader.Characteristics|=IMAGE_FILE_DLL;CHECK(!mr_executable_identity_supported(L"cnc3game.dat",image,sizeof(image)));nt->FileHeader.Characteristics&=~IMAGE_FILE_DLL;
 nt->OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR64_MAGIC;CHECK(!mr_executable_identity_supported(L"cnc3game.dat",image,sizeof(image)));nt->OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR32_MAGIC;
 dos->e_lfanew=4095;CHECK(!mr_executable_identity_supported(L"cnc3game.dat",image,sizeof(image)));dos->e_lfanew=-1;CHECK(!mr_executable_identity_supported(L"cnc3game.dat",image,sizeof(image)));
 CHECK(!mr_executable_verified());
 puts("PASS: TW identity accepts LAA/header changes without hashes and rejects unrelated, malformed, DLL and non-x86 hosts.");return 0;
}
