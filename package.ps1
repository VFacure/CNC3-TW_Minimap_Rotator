param([string]$ZigPath, [switch]$SkipBuild)
$ErrorActionPreference = 'Stop'
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -ZigPath $ZigPath }
$version = (Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'VERSION')).Trim()
$dll = Join-Path $PSScriptRoot 'build/dinput8.dll'
if (-not (Test-Path -LiteralPath $dll)) { throw 'Build the DLL first.' }
$dist = Join-Path $PSScriptRoot 'dist'
$staging = Join-Path $dist 'package'
New-Item -ItemType Directory -Path $staging -Force | Out-Null
Copy-Item -LiteralPath $dll -Destination (Join-Path $staging 'dinput8.dll') -Force
foreach ($name in @('README.md','LICENSE','THIRD-PARTY-NOTICES.txt')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $staging $name) -Force
}
$dllHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $dll).Hash.ToLowerInvariant()
[IO.File]::WriteAllText((Join-Path $staging 'SHA256SUMS.txt'), "$dllHash  dinput8.dll`n", [Text.Encoding]::ASCII)
$zip = Join-Path $dist "Minimap_Rotator-$version.zip"
$items = @('dinput8.dll','README.md','LICENSE','THIRD-PARTY-NOTICES.txt','SHA256SUMS.txt') |
    ForEach-Object { Join-Path $staging $_ }
Compress-Archive -LiteralPath $items -DestinationPath $zip -Force
$zipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zip).Hash.ToLowerInvariant()
[IO.File]::WriteAllText((Join-Path $dist 'SHA256SUMS.txt'), "$zipHash  Minimap_Rotator-$version.zip`n$dllHash  dinput8.dll`n", [Text.Encoding]::ASCII)
Get-Item -LiteralPath $zip
