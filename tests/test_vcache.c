#include <stdio.h>
#include "../source/vcache.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static int calls,verdict=1;
static int fake(const void *p,SIZE_T n){(void)p;(void)n;calls++;return verdict;}
int main(void){
 char a[64],b[64];MRVCache c={.validate=fake};
 /* First query validates; the next 28 frames reuse the cached result. */
 mr_vc_begin_frame(&c);CHECK(mr_vc_readable(&c,0,a,4)&&calls==1);
 for(unsigned f=1;f<MR_VCACHE_INTERVAL-1;f++){mr_vc_begin_frame(&c);CHECK(mr_vc_readable(&c,0,a,4));}
 CHECK(calls==1);
 /* The 30th frame drops the cache and re-validates. */
 mr_vc_begin_frame(&c);CHECK(mr_vc_readable(&c,0,a,4)&&calls==2);
 /* A changed pointer value is validated at once, even mid-interval. */
 mr_vc_begin_frame(&c);CHECK(mr_vc_readable(&c,0,b,4)&&calls==3);
 CHECK(mr_vc_readable(&c,0,b,4)&&calls==3);
 /* A larger size than proved is validated again; a smaller one is not. */
 CHECK(mr_vc_readable(&c,0,b,32)&&calls==4);
 CHECK(mr_vc_readable(&c,0,b,8)&&calls==4);
 /* Slots are independent. */
 CHECK(mr_vc_readable(&c,1,b,8)&&calls==5);
 /* Failures are never cached: every query re-validates until it passes. */
 verdict=0;CHECK(!mr_vc_readable(&c,2,a,4)&&calls==6);CHECK(!mr_vc_readable(&c,2,a,4)&&calls==7);
 verdict=1;CHECK(mr_vc_readable(&c,2,a,4)&&calls==8);
 /* A pointer that becomes unreadable after a pass fails on the re-validation. */
 mr_vc_reset(&c);CHECK(c.age==0&&!c.slot[0].ok&&!c.slot[2].ok);
 CHECK(mr_vc_readable(&c,0,a,4)&&calls==9);
 mr_vc_reset(&c);verdict=0;CHECK(!mr_vc_readable(&c,0,a,4)&&calls==10);
 /* Rebind/reset drops everything: the same pointer is validated again. */
 verdict=1;CHECK(mr_vc_readable(&c,0,a,4)&&calls==11);mr_vc_reset(&c);CHECK(mr_vc_readable(&c,0,a,4)&&calls==12);
 puts("vcache tests passed");return 0;
}
