param([string]$ZigPath, [switch]$SkipBuild)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'tools/common.ps1')
$cc = Get-ZigCompiler $ZigPath
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -ZigPath $cc }
$buildDir = Join-Path $PSScriptRoot 'build'
$dll = Join-Path $buildDir 'dinput8.dll'
if (-not (Test-Path -LiteralPath $dll)) { throw 'Build the DLL first.' }
function Run-Test {
    param([string]$Name, [string[]]$Files, [string[]]$Libraries = @(), [string[]]$RunArguments = @(), [string[]]$Defines = @())
    $program = Join-Path $buildDir "$Name.exe"
    & $cc cc @CompileFlags -DMR_TESTING @Defines @Files @Libraries -o $program
    if ($LASTEXITCODE -ne 0) { throw "$Name compilation failed." }
    & $program @RunArguments
    if ($LASTEXITCODE -ne 0) { throw "$Name failed ($LASTEXITCODE)." }
}
Run-Test -Name test_executable_guard -Files @((Join-Path $PSScriptRoot 'tests/test_executable_guard.c'), (Join-Path $PSScriptRoot 'source/executable_guard.c'))
Run-Test -Name test_transform -Files @(Join-Path $PSScriptRoot 'tests/test_transform.c')
Run-Test -Name test_proxy -Files @(Join-Path $PSScriptRoot 'tests/test_proxy.c') -RunArguments @($dll)
$startupFiles = @((Join-Path $PSScriptRoot 'tests/test_proxy_startup.c'), (Join-Path $PSScriptRoot 'source/dinput_proxy.c')) + $MinHookSources
Run-Test -Name test_proxy_startup -Files $startupFiles -Defines @('-DMR_ENABLE_GAME_HOOKS=1')
Run-Test -Name test_dll -Files @(Join-Path $PSScriptRoot 'tests/test_dll.c') -RunArguments @($dll)
Run-Test -Name test_radar_layout -Files @((Join-Path $PSScriptRoot 'tests/test_radar_layout.c'), (Join-Path $PSScriptRoot 'source/radar_layout.c'))
$gpuFiles = @((Join-Path $PSScriptRoot 'tests/test_composite.c'), (Join-Path $PSScriptRoot 'source/composite.c')) + $MinHookSources
Run-Test -Name test_composite -Files $gpuFiles -Libraries @('-ld3d9','-lgdi32')
