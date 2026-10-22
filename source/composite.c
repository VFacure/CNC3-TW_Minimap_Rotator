#define COBJMACROS
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "composite.h"
#include "camera_adapter.h"
#include "minimap_transform.h"
#include "../tools/minhook/include/MinHook.h"
typedef HRESULT (WINAPI *PresentFn)(IDirect3DDevice9*,const RECT*,const RECT*,HWND,const RGNDATA*);
typedef ULONG (WINAPI *ReleaseFn)(IDirect3DDevice9*);
typedef HRESULT (WINAPI *SwapPresentFn)(IDirect3DSwapChain9*,const RECT*,const RECT*,HWND,const RGNDATA*,DWORD);
typedef struct Device {void *target;PresentFn original;struct Device *next;} Device;
typedef struct Swap {void *target;SwapPresentFn original;struct Swap *next;} Swap;
static Device *devices;
static Swap *swaps;
static IDirect3DDevice9 *active_device;
static _Thread_local unsigned present_depth;
static SRWLOCK data_lock=SRWLOCK_INIT;
static struct {float x,y,w,h;double angle;int seen;} observed;
static struct {MRTransform t;double w,h,ox,oy;DWORD time;int active;} displayed;
static struct {double x,y,w,h;int valid;} layout;
static int enabled=1;
int mr_composite_enabled(void){return enabled;}
void mr_composite_observe(float x,float y,float w,float h){
 static int raw_logged,failed_logged;
 if(!raw_logged){mr_log("Radar surface observed: pos=(%.6f,%.6f) size=(%.6f,%.6f)",x,y,w,h);raw_logged=1;}
 double angle;
 if(!mr_camera_angle(&angle)){if(!failed_logged){mr_log("Camera-angle guard rejected the active camera");failed_logged=1;}return;}
 if(!isfinite(x)||!isfinite(y)||!isfinite(w)||!isfinite(h)||w<=0||h<=0)return;
 AcquireSRWLockExclusive(&data_lock);observed.x=x;observed.y=y;observed.w=w;observed.h=h;observed.angle=angle;observed.seen=1;ReleaseSRWLockExclusive(&data_lock);
}
void mr_composite_layout(double x,double y,double w,double h){
 if(!isfinite(x)||!isfinite(y)||!isfinite(w)||!isfinite(h)||w<1||h<1||w>20||h>20)return;
 if(x>0.01||y>0.01||x+w<0.99||y+h<0.99)return;
 AcquireSRWLockExclusive(&data_lock);layout.x=x;layout.y=y;layout.w=w;layout.h=h;layout.valid=1;ReleaseSRWLockExclusive(&data_lock);
}
void mr_input_remap(float *p){
 if(!p)return;AcquireSRWLockShared(&data_lock);
 if(displayed.active&&(DWORD)(GetTickCount()-displayed.time)<1000){
  MRPoint v={(double)p[0]*displayed.w+displayed.ox,(double)p[1]*displayed.h+displayed.oy};v=mr_inverse(&displayed.t,v);
  /* Empty triangular margins map to their nearest real map edge. */
  p[0]=(float)fmin(1,fmax(0,(v.x-displayed.ox)/displayed.w));p[1]=(float)fmin(1,fmax(0,(v.y-displayed.oy)/displayed.h));
 }
 ReleaseSRWLockShared(&data_lock);
}
typedef struct {float x,y,z,rhw;DWORD color;float u,v;} Vertex;
static void quad(Vertex *v,MRTransform *t,double ox,double oy,double w,double h,int rotate){
 const double uv[4][2]={{0,0},{1,0},{0,1},{1,1}};
 for(int i=0;i<4;i++){
  MRPoint p={uv[i][0]*w,uv[i][1]*h};if(rotate)p=mr_forward(t,p);
  v[i]=(Vertex){(float)(ox+p.x-0.5),(float)(oy+p.y-0.5),0,1,0xffffffff,(float)uv[i][0],(float)uv[i][1]};
 }
}
static HRESULT compose(IDirect3DDevice9 *d,RECT rect,double angle,MRTransform *transform){
 IDirect3DSurface9 *back=NULL,*image=NULL,*resolve=NULL,*old_rt=NULL,*old_depth=NULL;
 IDirect3DTexture9 *texture=NULL;IDirect3DStateBlock9 *state=NULL;HRESULT hr=E_FAIL;int scene=0,target_changed=0;
 D3DSURFACE_DESC desc;D3DVIEWPORT9 old_view;
 if(FAILED(IDirect3DDevice9_GetBackBuffer(d,0,0,D3DBACKBUFFER_TYPE_MONO,&back)))goto done;
 if(FAILED(IDirect3DSurface9_GetDesc(back,&desc)))goto done;
 if(rect.left<0||rect.top<0||rect.right>(LONG)desc.Width||rect.bottom>(LONG)desc.Height)goto done;
 UINT w=(UINT)(rect.right-rect.left),h=(UINT)(rect.bottom-rect.top);if(!w||!h)goto done;
 if(!mr_transform(transform,w,h,angle))goto done;
 if(FAILED(IDirect3DDevice9_CreateTexture(d,w,h,1,D3DUSAGE_RENDERTARGET,desc.Format,D3DPOOL_DEFAULT,&texture,NULL)))goto done;
 if(FAILED(IDirect3DTexture9_GetSurfaceLevel(texture,0,&image)))goto done;
 hr=IDirect3DDevice9_StretchRect(d,back,&rect,image,NULL,D3DTEXF_NONE);
 if(FAILED(hr)){
  if(FAILED(IDirect3DDevice9_CreateRenderTarget(d,desc.Width,desc.Height,desc.Format,D3DMULTISAMPLE_NONE,0,FALSE,&resolve,NULL)))goto done;
  if(FAILED(IDirect3DDevice9_StretchRect(d,back,NULL,resolve,NULL,D3DTEXF_NONE)))goto done;
  if(FAILED(IDirect3DDevice9_StretchRect(d,resolve,&rect,image,NULL,D3DTEXF_NONE)))goto done;
 }
 hr=E_FAIL;
 if(FAILED(IDirect3DDevice9_CreateStateBlock(d,D3DSBT_ALL,&state))||FAILED(IDirect3DStateBlock9_Capture(state)))goto done;
 if(FAILED(IDirect3DDevice9_GetRenderTarget(d,0,&old_rt))||FAILED(IDirect3DDevice9_GetViewport(d,&old_view)))goto done;
 IDirect3DDevice9_GetDepthStencilSurface(d,&old_depth);
 if(FAILED(IDirect3DDevice9_SetRenderTarget(d,0,back)))goto done;target_changed=1;
 IDirect3DDevice9_SetDepthStencilSurface(d,NULL);
 D3DVIEWPORT9 view={0,0,desc.Width,desc.Height,0,1};IDirect3DDevice9_SetViewport(d,&view);
 if(FAILED(IDirect3DDevice9_BeginScene(d)))goto done;scene=1;
 IDirect3DDevice9_SetPixelShader(d,NULL);IDirect3DDevice9_SetVertexShader(d,NULL);
 IDirect3DDevice9_SetFVF(d,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
 IDirect3DDevice9_SetTexture(d,0,(IDirect3DBaseTexture9*)texture);
 IDirect3DDevice9_SetTexture(d,1,NULL);
 #define RS(k,v) IDirect3DDevice9_SetRenderState(d,k,v)
 RS(D3DRS_ZENABLE,FALSE);RS(D3DRS_ZWRITEENABLE,FALSE);RS(D3DRS_ALPHABLENDENABLE,FALSE);
 RS(D3DRS_ALPHATESTENABLE,FALSE);RS(D3DRS_CULLMODE,D3DCULL_NONE);RS(D3DRS_LIGHTING,FALSE);
 RS(D3DRS_FOGENABLE,FALSE);RS(D3DRS_STENCILENABLE,FALSE);RS(D3DRS_SCISSORTESTENABLE,TRUE);
 RS(D3DRS_COLORWRITEENABLE,15);RS(D3DRS_SRGBWRITEENABLE,FALSE);RS(D3DRS_FILLMODE,D3DFILL_SOLID);
 RS(D3DRS_MULTISAMPLEANTIALIAS,FALSE);RS(D3DRS_CLIPPING,TRUE);RS(D3DRS_CLIPPLANEENABLE,0);
 #undef RS
 IDirect3DDevice9_SetScissorRect(d,&rect);
 IDirect3DDevice9_SetTextureStageState(d,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
 IDirect3DDevice9_SetTextureStageState(d,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
 IDirect3DDevice9_SetTextureStageState(d,0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
 IDirect3DDevice9_SetTextureStageState(d,0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
 IDirect3DDevice9_SetTextureStageState(d,0,D3DTSS_TEXCOORDINDEX,0);
 IDirect3DDevice9_SetTextureStageState(d,0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
 IDirect3DDevice9_SetTextureStageState(d,1,D3DTSS_COLOROP,D3DTOP_DISABLE);
 IDirect3DDevice9_SetTextureStageState(d,1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
 IDirect3DDevice9_SetSamplerState(d,0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
 IDirect3DDevice9_SetSamplerState(d,0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
 IDirect3DDevice9_SetSamplerState(d,0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
 IDirect3DDevice9_SetSamplerState(d,0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
 IDirect3DDevice9_SetSamplerState(d,0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
 IDirect3DDevice9_SetSamplerState(d,0,D3DSAMP_SRGBTEXTURE,FALSE);
 D3DRECT clear={rect.left,rect.top,rect.right,rect.bottom};
 if(FAILED(IDirect3DDevice9_Clear(d,1,&clear,D3DCLEAR_TARGET,0xff000000,1,0)))goto done;
 Vertex vertices[4];quad(vertices,transform,rect.left,rect.top,w,h,1);
 hr=IDirect3DDevice9_DrawPrimitiveUP(d,D3DPT_TRIANGLESTRIP,2,vertices,sizeof(Vertex));
 done:
 if(scene)IDirect3DDevice9_EndScene(d);
 if(FAILED(hr)&&scene&&image&&back)IDirect3DDevice9_StretchRect(d,image,NULL,back,&rect,D3DTEXF_NONE);
 if(target_changed){IDirect3DDevice9_SetRenderTarget(d,0,old_rt);IDirect3DDevice9_SetDepthStencilSurface(d,old_depth);IDirect3DDevice9_SetViewport(d,&old_view);}
 if(state){IDirect3DStateBlock9_Apply(state);IDirect3DStateBlock9_Release(state);}
 if(old_depth)IDirect3DSurface9_Release(old_depth);if(old_rt)IDirect3DSurface9_Release(old_rt);
 if(resolve)IDirect3DSurface9_Release(resolve);if(image)IDirect3DSurface9_Release(image);
 if(texture)IDirect3DTexture9_Release(texture);if(back)IDirect3DSurface9_Release(back);
 return hr;
}
static Device *find_device(IDirect3DDevice9 *d){for(Device *n=devices;n;n=n->next)if(n->target==(void*)d->lpVtbl->Present)return n;return NULL;}
static Swap *find_swap(IDirect3DSwapChain9 *s){for(Swap *n=swaps;n;n=n->next)if(n->target==(void*)s->lpVtbl->Present)return n;return NULL;}
static void render_frame(IDirect3DDevice9 *d){
 if(d!=active_device)return;
 float x,y,w,h;double a;int seen;
 AcquireSRWLockExclusive(&data_lock);x=observed.x;y=observed.y;w=observed.w;h=observed.h;a=observed.angle;seen=observed.seen;observed.seen=0;ReleaseSRWLockExclusive(&data_lock);
 if(!seen)return;
 MRTransform transform;HRESULT result=E_FAIL;RECT rect={0},window={0};
 IDirect3DSurface9 *back=NULL;D3DSURFACE_DESC desc;
 if(SUCCEEDED(IDirect3DDevice9_GetBackBuffer(d,0,0,D3DBACKBUFFER_TYPE_MONO,&back))){
  if(SUCCEEDED(IDirect3DSurface9_GetDesc(back,&desc))){
   if(w<=2&&h<=2&&fabs(x)<=2&&fabs(y)<=2){x*=desc.Width;w*=desc.Width;y*=desc.Height;h*=desc.Height;}
   rect=(RECT){(LONG)lround(x),(LONG)lround(y),(LONG)lround(x+w),(LONG)lround(y+h)};
   if(w>=16&&h>=16&&w<=desc.Width&&h<=desc.Height){
    double lx,ly,lw,lh;int valid;
    AcquireSRWLockShared(&data_lock);lx=layout.x;ly=layout.y;lw=layout.w;lh=layout.h;valid=layout.valid;ReleaseSRWLockShared(&data_lock);
    if(!valid){double side=fmax(w,h);lx=(w-side)/(2*w);ly=(h-side)/(2*h);lw=side/w;lh=side/h;}
    window=(RECT){(LONG)lround(x+lx*w),(LONG)lround(y+ly*h),(LONG)lround(x+(lx+lw)*w),(LONG)lround(y+(ly+lh)*h)};
    result=compose(d,window,a,&transform);
   }
  }IDirect3DSurface9_Release(back);
 }
 AcquireSRWLockExclusive(&data_lock);displayed.active=SUCCEEDED(result);
 if(displayed.active){displayed.t=transform;displayed.w=rect.right-rect.left;displayed.h=rect.bottom-rect.top;displayed.ox=rect.left-window.left;displayed.oy=rect.top-window.top;displayed.time=GetTickCount();}
 ReleaseSRWLockExclusive(&data_lock);
 static int logged;
 if(!logged){mr_log("Composite first frame: map=(%ld,%ld)-(%ld,%ld), window=(%ld,%ld)-(%ld,%ld), angle=%.5f, result=%08lx",rect.left,rect.top,rect.right,rect.bottom,window.left,window.top,window.right,window.bottom,a,(unsigned long)result);logged=1;}
}
static HRESULT WINAPI present(IDirect3DDevice9 *d,const RECT *src,const RECT *dst,HWND wnd,const RGNDATA *dirty){
 Device *n=find_device(d);if(!n)return D3DERR_INVALIDCALL;
 static int logged;if(!logged){mr_log("Device Present detour entered");logged=1;}
 if(present_depth++==0)render_frame(d);
 HRESULT hr=n->original(d,src,dst,wnd,dirty);present_depth--;return hr;
}
static HRESULT WINAPI swap_present(IDirect3DSwapChain9 *s,const RECT *src,const RECT *dst,HWND wnd,const RGNDATA *dirty,DWORD flags){
 Swap *n=find_swap(s);if(!n)return D3DERR_INVALIDCALL;
 static int logged;if(!logged){mr_log("Swap-chain Present detour entered");logged=1;}
 IDirect3DDevice9 *d=NULL;
 if(present_depth++==0&&SUCCEEDED(IDirect3DSwapChain9_GetDevice(s,&d))){render_frame(d);IDirect3DDevice9_Release(d);}
 HRESULT hr=n->original(s,src,dst,wnd,dirty,flags);present_depth--;return hr;
}
#ifdef MR_TESTING
HRESULT mr_test_compose(IDirect3DDevice9 *d,RECT rect,double angle,MRTransform *t){return compose(d,rect,angle,t);}
#endif
static int attach_swap(IDirect3DDevice9 *d){
 IDirect3DSwapChain9 *s=NULL;if(FAILED(IDirect3DDevice9_GetSwapChain(d,0,&s)))return 0;
 if(find_swap(s)){IDirect3DSwapChain9_Release(s);return 1;}
 Swap *n=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*n));if(!n){IDirect3DSwapChain9_Release(s);return 0;}
 n->target=(void*)s->lpVtbl->Present;IDirect3DSwapChain9_Release(s);
 MH_STATUS status=MH_CreateHook(n->target,(void*)&swap_present,(void**)&n->original);
 if(status!=MH_OK){mr_log("Swap Present create failed: %s",MH_StatusToString(status));HeapFree(GetProcessHeap(),0,n);return 0;}
 n->next=swaps;swaps=n;status=MH_EnableHook(n->target);
 if(status!=MH_OK){swaps=n->next;MH_RemoveHook(n->target);HeapFree(GetProcessHeap(),0,n);mr_log("Swap Present enable failed: %s",MH_StatusToString(status));return 0;}
 mr_log("Swap Present function detoured at %p",n->target);return 1;
}
int mr_composite_attach(IDirect3DDevice9 *d){
 if(!d)return 0;active_device=d;
 MH_STATUS status=MH_Initialize();if(status!=MH_OK&&status!=MH_ERROR_ALREADY_INITIALIZED)return 0;
 if(!find_device(d)){
  Device *n=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*n));if(!n)return 0;
  n->target=(void*)d->lpVtbl->Present;status=MH_CreateHook(n->target,(void*)&present,(void**)&n->original);
  if(status!=MH_OK){mr_log("Device Present create failed: %s",MH_StatusToString(status));HeapFree(GetProcessHeap(),0,n);return 0;}
  n->next=devices;devices=n;status=MH_EnableHook(n->target);
  if(status!=MH_OK){devices=n->next;MH_RemoveHook(n->target);HeapFree(GetProcessHeap(),0,n);mr_log("Device Present enable failed: %s",MH_StatusToString(status));return 0;}
  mr_log("Device Present function detoured at %p",n->target);
 }
 return attach_swap(d);
}
