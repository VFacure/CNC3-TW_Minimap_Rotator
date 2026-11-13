$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$cc=Join-Path $PSScriptRoot 'compiler\zig-windows-x86_64-0.13.0\zig.exe'
$out=Join-Path $root 'build\test_transform.exe'
& $cc cc -target x86-windows-gnu -std=c11 -O2 (Join-Path $PSScriptRoot 'test_transform.c') -o $out
if($LASTEXITCODE -ne 0){throw 'Transform compile failed'}
$result=& $out
if($LASTEXITCODE -ne 0){throw 'Transform tests failed'}
$result | Set-Content (Join-Path $root 'reports\transform-test.txt')
$result
