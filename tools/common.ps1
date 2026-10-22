function Get-ZigCompiler([string]$ZigPath) {
    if (-not $ZigPath) { $ZigPath = $env:ZIG_EXE }
    if (-not $ZigPath) {
        $command = Get-Command zig -ErrorAction SilentlyContinue
        if ($command) { $ZigPath = $command.Source }
    }
    if (-not $ZigPath -or -not (Test-Path -LiteralPath $ZigPath)) {
        throw 'Install Zig 0.13.0 and pass -ZigPath, set ZIG_EXE, or add zig to PATH.'
    }
    $version = & $ZigPath version
    if ($LASTEXITCODE -ne 0 -or $version -ne '0.13.0') { throw 'Zig 0.13.0 is required.' }
    return (Resolve-Path -LiteralPath $ZigPath).Path
}
$CompileFlags = @('-target','x86-windows-gnu','-std=c11','-Wall','-Wextra','-Werror',
                  '-Wno-unused-function','-Wno-unused-parameter','-O2')
$MinHookSources = @('src/hook.c','src/buffer.c','src/trampoline.c','src/hde/hde32.c') |
    ForEach-Object { Join-Path $PSScriptRoot "minhook/$_" }
