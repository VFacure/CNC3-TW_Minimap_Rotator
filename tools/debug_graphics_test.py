"""Run only the project's standalone graphics test under the Win32 debugger."""
import ctypes as C,json,time
from ctypes import wintypes as W
from pathlib import Path
R=Path(__file__).resolve().parents[1];k=C.WinDLL('kernel32',use_last_error=True)
class ER(C.Structure):_fields_=[('code',W.DWORD),('flags',W.DWORD),('record',C.c_void_p),('address',C.c_void_p),('count',W.DWORD),('info',C.c_size_t*15)]
class EI(C.Structure):_fields_=[('record',ER),('first',W.DWORD)]
class U(C.Union):_fields_=[('exception',EI),('raw',C.c_byte*160)]
class Event(C.Structure):_fields_=[('code',W.DWORD),('pid',W.DWORD),('tid',W.DWORD),('u',U)]
class SI(C.Structure):_fields_=[('cb',W.DWORD),('reserved',W.LPWSTR),('desktop',W.LPWSTR),('title',W.LPWSTR),('x',W.DWORD),('y',W.DWORD),('w',W.DWORD),('h',W.DWORD),('xc',W.DWORD),('yc',W.DWORD),('fill',W.DWORD),('flags',W.DWORD),('show',W.WORD),('res2',W.WORD),('resptr',C.c_void_p),('stdin',W.HANDLE),('stdout',W.HANDLE),('stderr',W.HANDLE)]
class PI(C.Structure):_fields_=[('process',W.HANDLE),('thread',W.HANDLE),('pid',W.DWORD),('tid',W.DWORD)]
k.CreateProcessW.argtypes=[W.LPCWSTR,W.LPWSTR,C.c_void_p,C.c_void_p,W.BOOL,W.DWORD,C.c_void_p,W.LPCWSTR,C.POINTER(SI),C.POINTER(PI)];k.CreateProcessW.restype=W.BOOL
k.WaitForDebugEvent.argtypes=[C.POINTER(Event),W.DWORD];k.ContinueDebugEvent.argtypes=[W.DWORD,W.DWORD,W.DWORD]
k.TerminateProcess.argtypes=[W.HANDLE,W.UINT];k.CloseHandle.argtypes=[W.HANDLE]
si=SI();si.cb=C.sizeof(si);pi=PI();exe=R/'build/test_composite.exe'
if not k.CreateProcessW(str(exe),None,None,None,False,2|0x08000000,None,str(R/'build'),C.byref(si),C.byref(pi)):raise C.WinError(C.get_last_error())
rows=[];start=time.monotonic()
try:
 while time.monotonic()-start<60:
  e=Event()
  if not k.WaitForDebugEvent(C.byref(e),1000):continue
  status=0x10002
  if e.code==1:
   x=e.u.exception;rows.append(dict(exception=hex(x.record.code),address=hex(x.record.address or 0),first=bool(x.first)))
   if x.record.code!=0x80000003:status=0x80010001
   if not x.first:
    k.OpenThread.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenThread.restype=W.HANDLE
    th=k.OpenThread(0x48,False,e.tid);buf=C.create_string_buffer(716);C.c_uint32.from_buffer(buf).value=0x10003
    k.Wow64GetThreadContext.argtypes=[W.HANDLE,C.c_void_p]
    if k.Wow64GetThreadContext(th,buf):
     import struct
     regs={n:hex(struct.unpack_from('<I',buf,o)[0]) for n,o in [('eax',176),('ecx',172),('edx',168),('ebp',180),('eip',184),('esp',196)]}
     sp=int(regs['esp'],16);stack=C.create_string_buffer(80);got=C.c_size_t()
     k.ReadProcessMemory.argtypes=[W.HANDLE,C.c_void_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t)]
     k.ReadProcessMemory(pi.process,sp,stack,80,C.byref(got));regs['stack']=[hex(v) for v in struct.unpack('<20I',stack)]
     rows.append(regs)
    k.CloseHandle(th);break
  if e.code==5:
   rows.append({'exit':int.from_bytes(bytes(e.u.raw[:4]),'little')});k.ContinueDebugEvent(e.pid,e.tid,status);break
  k.ContinueDebugEvent(e.pid,e.tid,status)
finally:
 k.TerminateProcess(pi.process,99);k.CloseHandle(pi.thread);k.CloseHandle(pi.process)
(R/'reports/graphics-debug.json').write_text(json.dumps(rows,indent=2));print(json.dumps(rows,indent=2))

