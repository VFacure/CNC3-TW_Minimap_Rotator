#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "../source/composite.h"
#include "../source/minimap_transform.h"
HRESULT mr_test_compose(IDirect3DDevice9*,RECT,double,MRTransform*);
static double fake_angle;
int mr_camera_angle(double *a){*a=fake_angle;return 1;}
void mr_log(const char *format,...){va_list a;va_start(a,format);vprintf(format,a);putchar('\n');va_end(a);}
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static int paint(IDirect3DDevice9 *d,RECT r){
 if(FAILED(IDirect3DDevice9_Clear(d,0,NULL,D3DCLEAR_TARGET,0xff303030,1,0)))return 0;
 LONG cx=(r.left+r.right)/2,cy=(r.top+r.bottom)/2;
 D3DRECT q[4]={{r.left,r.top,cx,cy},{cx,r.top,r.right,cy},{r.left,cy,cx,r.bottom},{cx,cy,r.right,r.bottom}};
 DWORD colors[]={0xffff0000,0xff00ff00,0xff0000ff,0xffffff00};
 for(int i=0;i<4;i++)if(FAILED(IDirect3DDevice9_Clear(d,1,q+i,D3DCLEAR_TARGET,colors[i],1,0)))return 0;
 return 1;
}
int main(void){setvbuf(stdout,NULL,_IONBF,0);puts("Starting graphics test");
 HINSTANCE inst=GetModuleHandleW(NULL);WNDCLASSW wc={0};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=inst;wc.lpszClassName=L"MRGraphicsTest";
 CHECK(RegisterClassW(&wc));HWND hwnd=CreateWindowExW(0,wc.lpszClassName,L"Minimap offline graphics test",WS_POPUP,0,0,320,240,NULL,NULL,inst,NULL);CHECK(hwnd);
 IDirect3D9 *api=Direct3DCreate9(D3D_SDK_VERSION);CHECK(api);
 D3DPRESENT_PARAMETERS pp={0};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=hwnd;pp.BackBufferWidth=320;pp.BackBufferHeight=240;pp.BackBufferFormat=D3DFMT_X8R8G8B8;pp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
 IDirect3DDevice9 *d=NULL;HRESULT hr=IDirect3D9_CreateDevice(api,D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&d);printf("CreateDevice: %08lx\n",(unsigned long)hr);CHECK(SUCCEEDED(hr));
 IDirect3DSurface9 *back=NULL,*read=NULL;CHECK(SUCCEEDED(IDirect3DDevice9_GetBackBuffer(d,0,0,D3DBACKBUFFER_TYPE_MONO,&back)));
 CHECK(SUCCEEDED(IDirect3DDevice9_CreateOffscreenPlainSurface(d,320,240,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM,&read,NULL)));
 const RECT rectangles[]={{64,48,192,176},{64,48,224,144},{64,48,192,176},{64,48,224,144}};
 const double angles[]={90,45,0,-315};const DWORD colors[]={0xff0000,0x00ff00,0x0000ff,0xffff00};
 for(unsigned k=0;k<4;k++){
  RECT r=rectangles[k];double w=r.right-r.left,h=r.bottom-r.top;CHECK(paint(d,r));
  CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderState(d,D3DRS_ALPHABLENDENABLE,TRUE)));
  MRTransform t;hr=mr_test_compose(d,r,angles[k]*acos(-1)/180,&t);printf("compose %u: %08lx\n",k,(unsigned long)hr);CHECK(SUCCEEDED(hr));
  DWORD blend=0;CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderState(d,D3DRS_ALPHABLENDENABLE,&blend)));CHECK(blend==TRUE);
  CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(d,back,read)));
  D3DLOCKED_RECT pixels;CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(read,&pixels,NULL,D3DLOCK_READONLY)));
  for(int q=0;q<4;q++){
   MRPoint p={w*((q%2)?0.75:0.25),h*((q/2)?0.75:0.25)};p=mr_forward(&t,p);
   int x=(int)lround(r.left+p.x),y=(int)lround(r.top+p.y);
   DWORD actual=*(DWORD*)((BYTE*)pixels.pBits+y*pixels.Pitch+x*4)&0xffffff;
   if(actual!=colors[q])printf("pixel (%d,%d): actual=%06lx expected=%06lx\n",x,y,(unsigned long)actual,(unsigned long)colors[q]);
   CHECK(actual==colors[q]);
  }
  CHECK(t.scale==1.0);
  for(int py=0;py<240;py++)for(int px=0;px<320;px++){
   if(px<r.left||px>=r.right||py<r.top||py>=r.bottom)
    CHECK((*(DWORD*)((BYTE*)pixels.pBits+py*pixels.Pitch+px*4)&0xffffff)==0x303030);
  }
  CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(read)));
 }
 typedef HRESULT(WINAPI *CachedPresent)(IDirect3DDevice9*,const RECT*,const RECT*,HWND,const RGNDATA*);
 CachedPresent cached=d->lpVtbl->Present;
 IDirect3DSwapChain9 *swap=NULL;CHECK(SUCCEEDED(IDirect3DDevice9_GetSwapChain(d,0,&swap)));
 typedef HRESULT(WINAPI *CachedSwap)(IDirect3DSwapChain9*,const RECT*,const RECT*,HWND,const RGNDATA*,DWORD);
 CachedSwap cached_swap=swap->lpVtbl->Present;
 mr_composite_layout(0,0,1,1);CHECK(mr_composite_attach(d));CHECK(paint(d,rectangles[1]));fake_angle=acos(-1)/4;
 mr_composite_observe(64,48,160,96);CHECK(SUCCEEDED(cached(d,NULL,NULL,NULL,NULL)));
 MRTransform t;CHECK(mr_transform(&t,160,96,fake_angle));MRPoint p={40,24},screen=mr_forward(&t,p);
 float normalized[2]={(float)(screen.x/160),(float)(screen.y/96)};mr_input_remap(normalized);
 CHECK(fabs(normalized[0]-0.25)<1e-5&&fabs(normalized[1]-0.25)<1e-5);
 CHECK(paint(d,rectangles[1]));mr_composite_observe(64,48,160,96);CHECK(SUCCEEDED(cached_swap(swap,NULL,NULL,NULL,NULL,0)));
 normalized[0]=(float)(screen.x/160);normalized[1]=(float)(screen.y/96);mr_input_remap(normalized);
 CHECK(fabs(normalized[0]-0.25)<1e-5&&fabs(normalized[1]-0.25)<1e-5);IDirect3DSwapChain9_Release(swap);
 /* Rectangular map can now occupy the former letterbox bars. */
 RECT map={64,80,224,176},window={64,48,224,208};
 CHECK(paint(d,map));D3DRECT bars[2]={{64,48,224,80},{64,176,224,208}};
 CHECK(SUCCEEDED(IDirect3DDevice9_Clear(d,2,bars,D3DCLEAR_TARGET,0xff000000,1,0)));
 CHECK(SUCCEEDED(mr_test_compose(d,window,acos(-1)/2,&t)));
 CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(d,back,read)));
 D3DLOCKED_RECT full;CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(read,&full,NULL,D3DLOCK_READONLY)));
 CHECK((*(DWORD*)((BYTE*)full.pBits+56*full.Pitch+140*4)&0xffffff)==0x0000ff);
 CHECK((*(DWORD*)((BYTE*)full.pBits+200*full.Pitch+148*4)&0xffffff)==0x00ff00);
 for(int yy=0;yy<240;yy++)for(int xx=0;xx<320;xx++)if(xx<64||xx>=224||yy<48||yy>=208)
  CHECK((*(DWORD*)((BYTE*)full.pBits+yy*full.Pitch+xx*4)&0xffffff)==0x303030);
 CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(read)));
 CHECK(paint(d,map));CHECK(SUCCEEDED(IDirect3DDevice9_Clear(d,2,bars,D3DCLEAR_TARGET,0xff000000,1,0)));
 fake_angle=acos(-1)/2;mr_composite_layout(0,-32.0/96,1,160.0/96);
 mr_composite_observe(64,80,160,96);CHECK(SUCCEEDED(cached(d,NULL,NULL,NULL,NULL)));
 float former_bar[2]={(140.0f-64)/160,(56.0f-80)/96};mr_input_remap(former_bar);
 CHECK(fabs(former_bar[0]-0.05)<1e-5&&fabs(former_bar[1]-(132.0-80)/96)<1e-5);
 mr_composite_layout(0,0,1,1);
 IDirect3DSurface9_Release(back);back=NULL;CHECK(SUCCEEDED(IDirect3DDevice9_Reset(d,&pp)));CHECK(SUCCEEDED(IDirect3DDevice9_GetBackBuffer(d,0,0,D3DBACKBUFFER_TYPE_MONO,&back)));CHECK(paint(d,rectangles[0]));mr_composite_observe(64,48,128,128);CHECK(SUCCEEDED(IDirect3DDevice9_Present(d,NULL,NULL,NULL,NULL)));
 IDirect3DSurface9_Release(read);IDirect3DSurface9_Release(back);IDirect3DDevice9_Release(d);IDirect3D9_Release(api);DestroyWindow(hwnd);
 puts("PASS: GPU fixed-scale/cropped rotation at four angles/aspects, every surrounding pixel, graphics state, cached device/swap-chain presentation, inverse input including former black bars, expanded viewport clipping, device reset. No game used.");return 0;
}
