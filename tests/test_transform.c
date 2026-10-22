#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../source/minimap_transform.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 MRTransform t;unsigned checks=0;
 assert(!mr_transform(&t,0,128,0));assert(!mr_transform(&t,128,128,NAN));
 double sizes[][2]={{128,128},{256,128},{128,256},{181,182}};
 for(unsigned k=0;k<4;k++)for(int deg=-720;deg<=720;deg++){
  double w=sizes[k][0],h=sizes[k][1];assert(mr_transform(&t,w,h,deg*acos(-1)/180));
  for(int y=0;y<=4;y++)for(int x=0;x<=4;x++){
   MRPoint p={w*x/4,h*y/4},q=mr_forward(&t,p),r=mr_inverse(&t,q);
   assert(fabs(r.x-p.x)<1e-9&&fabs(r.y-p.y)<1e-9);
   assert(t.scale==1.0);
   assert(fabs(hypot(q.x-t.cx,q.y-t.cy)-hypot(p.x-t.cx,p.y-t.cy))<1e-9);
   checks++;
  }
 }
 mr_transform(&t,128,128,acos(-1)/2);MRPoint q=mr_forward(&t,(MRPoint){128,64});
 assert(fabs(q.x-64)<1e-9&&fabs(q.y-128)<1e-9);
 mr_transform(&t,170,110,acos(-1)/4);q=mr_forward(&t,(MRPoint){170,0});
 assert(q.x>170 || q.y<0 || q.y>110); /* Cropping is intentional. */
 printf("PASS: %u inverse/fixed-scale cases; invalid inputs and 90-degree orientation. Engine integration NOT tested.\n",checks);
 return 0;
}
