#ifndef MR_VCACHE_H
#define MR_VCACHE_H
/* Cache of memory-readability checks (VirtualQuery is slow on the game's huge regions).
   A positive result is reused only while the pointer value is unchanged and fewer than
   MR_VCACHE_INTERVAL frames passed since it was proved. Negative results are never cached. */
#include <windows.h>
#define MR_VCACHE_INTERVAL 30u
#define MR_VCACHE_SLOTS 4
typedef int (*MRValidateFn)(const void *p, SIZE_T bytes);
typedef struct {const void *p; SIZE_T bytes; int ok;} MRCachedPtr;
typedef struct {MRCachedPtr slot[MR_VCACHE_SLOTS]; unsigned age; unsigned checks; MRValidateFn validate;} MRVCache;
static inline void mr_vc_reset(MRVCache *c){
 for(int i=0;i<MR_VCACHE_SLOTS;i++){c->slot[i].p=NULL;c->slot[i].bytes=0;c->slot[i].ok=0;}
 c->age=0;
}
/* Call once per frame (per camera-angle query). Drops every cached result each interval. */
static inline void mr_vc_begin_frame(MRVCache *c){if(++c->age>=MR_VCACHE_INTERVAL)mr_vc_reset(c);}
static inline int mr_vc_readable(MRVCache *c,int slot,const void *p,SIZE_T bytes){
 MRCachedPtr *s=&c->slot[slot];
 if(s->ok&&s->p==p&&bytes<=s->bytes)return 1;
 int r=c->validate(p,bytes);c->checks++;
 if(r){s->p=p;s->bytes=bytes;s->ok=1;}else s->ok=0;
 return r;
}
#endif
