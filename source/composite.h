#ifndef MR_COMPOSITE_H
#define MR_COMPOSITE_H
#include <d3d9.h>
int mr_composite_attach(IDirect3DDevice9 *device);
void mr_composite_observe(float x,float y,float w,float h);
void mr_input_remap(float *normalized_xy);
int mr_composite_enabled(void);
void mr_composite_layout(double x,double y,double width,double height);
void mr_log(const char *format,...);
#endif
