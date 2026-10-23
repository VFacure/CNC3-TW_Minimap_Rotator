#ifndef MR_EXECUTABLE_GUARD_H
#define MR_EXECUTABLE_GUARD_H
#include <windows.h>
int mr_executable_identity_supported(const WCHAR *path,const BYTE *headers,SIZE_T available);
int mr_executable_verified(void);
#endif
