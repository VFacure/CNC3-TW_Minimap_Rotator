$ErrorActionPreference='Stop'
$r=Split-Path $PSScriptRoot -Parent
$cc=Join-Path $PSScriptRoot 'compiler\zig-windows-x86_64-0.13.0\zig.exe'
$sources=@('extension.c','executable_guard.c','camera_adapter.c','composite.c','radar_layout.c','runtime.c','log.c','dinput_proxy.c','notices.c','dinput8.def') | ForEach-Object {Join-Path "$r\source" $_}
# Candidate: completed-image rotation with device and swap-chain detours.
$sources += @('src/hook.c','src/buffer.c','src/trampoline.c','src/hde/hde32.c') | ForEach-Object {Join-Path "$r\tools\minhook" $_}
& $cc cc -target x86-windows-gnu -std=c11 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -O2 -DMR_ENABLE_GAME_HOOKS=1 -shared @sources -o "$r\build\dinput8-dev.dll"
if($LASTEXITCODE -ne 0){throw 'DLL-only development build failed'}
Get-FileHash -LiteralPath "$r\build\dinput8-dev.dll" -Algorithm SHA256



