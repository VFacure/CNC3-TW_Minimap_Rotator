/* Pixel-space transform shared by visual layers and inverse mouse mapping.
   Engine integration must supply the absolute camera-relative angle. */
#ifndef MINIMAP_TRANSFORM_H
#define MINIMAP_TRANSFORM_H
#include <math.h>
typedef struct { double x,y; } MRPoint;
typedef struct { double cx,cy,c,s,scale; } MRTransform;
static int mr_transform(MRTransform *t,double w,double h,double angle) {
 if(!t || !isfinite(w)||!isfinite(h)||!isfinite(angle)||w<=0||h<=0)return 0;
 t->cx=w/2;t->cy=h/2;t->c=cos(angle);t->s=sin(angle);
 /* Keep map detail at its native pixel scale; the compositor scissors edges. */
 t->scale=1.0;return 1;
}
static MRPoint mr_forward(const MRTransform *t,MRPoint p){
 double x=p.x-t->cx,y=p.y-t->cy;
 return (MRPoint){t->cx+t->scale*(t->c*x-t->s*y),t->cy+t->scale*(t->s*x+t->c*y)};
}
static MRPoint mr_inverse(const MRTransform *t,MRPoint p){
 double x=(p.x-t->cx)/t->scale,y=(p.y-t->cy)/t->scale;
 return (MRPoint){t->cx+t->c*x+t->s*y,t->cy-t->s*x+t->c*y};
}
#endif
