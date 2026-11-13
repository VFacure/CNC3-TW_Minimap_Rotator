#ifndef MR_HOOK_H
#define MR_HOOK_H
#include <windows.h>
/* x86 only, six-byte prologue verified to contain no relative instructions.
   Install/remove only while all possible target callers are quiescent.
   This is deliberately not a general-purpose instruction relocator. */
typedef struct { BYTE *target,*trampoline; BYTE original[6],patch[6]; int active; } MRHook;
int mr_hook_install(MRHook*,void *target,void *replacement,const BYTE expected[6]);
int mr_hook_remove(MRHook*);
#endif
