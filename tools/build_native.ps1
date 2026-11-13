$ErrorActionPreference='Stop'
$r=Split-Path $PSScriptRoot -Parent
$cc=Join-Path $PSScriptRoot 'compiler\zig-windows-x86_64-0.13.0\zig.exe'
$sources=@('extension.c','executable_guard.c','camera_adapter.c','apt_bridge.c','radar_tick.c','hook.c') | ForEach-Object {Join-Path "$r\source" $_}
& $cc cc -target x86-windows-gnu -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O2 -shared @sources -lbcrypt  -o "$r\build\MinimapRotator-dev.dll"
if($LASTEXITCODE -ne 0){throw 'Native development DLL build failed'}
Get-FileHash -LiteralPath "$r\build\MinimapRotator-dev.dll" -Algorithm SHA256


