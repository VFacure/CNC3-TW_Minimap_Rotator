param([string]$ZigPath)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'tools/common.ps1')
$cc = Get-ZigCompiler $ZigPath
$sources = @('extension.c','executable_guard.c','camera_adapter.c','composite.c',
             'radar_layout.c','runtime.c','log.c','dinput_proxy.c','notices.c','dinput8.def') |
    ForEach-Object { Join-Path $PSScriptRoot "source/$_" }
$buildDir = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
$dll = Join-Path $buildDir 'dinput8.dll'
& $cc cc @CompileFlags -DMR_ENABLE_GAME_HOOKS=1 -shared @sources @MinHookSources -o $dll
if ($LASTEXITCODE -ne 0) { throw 'DLL build failed.' }
Get-FileHash -LiteralPath $dll -Algorithm SHA256
