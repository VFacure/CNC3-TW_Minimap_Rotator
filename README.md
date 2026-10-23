# CNC3:TW Minimap Rotator

Rotates the minimap with the camera when you hold **Alt** in Command & Conquer 3: Tiberium Wars. Terrain, fog, unit icons and the camera indicator rotate together. The scale stays fixed, rectangular maps use the full minimap window, and clicks follow the rotated map.

## Install

1. Close the game and download the ZIP from [Releases](https://github.com/VFacure/CNC3-TW_Minimap_Rotator/releases/latest).
2. Extract `dinput8.dll` into the game's `RetailExe/1.9` folder, beside `cnc3game.dat`.
3. Start the game normally.

To uninstall, close the game and remove that DLL. No BIG file or separate injector is needed. If another mod or wrapper already supplies `dinput8.dll`, do not overwrite it; the two need a compatibility solution.

## What the DLL does

The game loads this DLL as a DirectInput proxy. It forwards DirectInput calls to the original Windows library, reads the camera orientation, rotates the finished Direct3D 9 minimap image, and reverses that rotation for mouse input.

MinHook installs hooks inside the game process. It does not patch the executable or mod archives on disk, run a background service, or request administrator access. Its only runtime file output is `MinimapRotator.log` beside the DLL. The mod contains no network or telemetry code.

The DLL is unsigned. Source, build scripts and SHA-256 checksums are provided so the release can be inspected and checked.

## Compatibility

Recognizes the x86 Tiberium Wars engine (`cnc3game.dat` or `cnc3game.exe`) without an executable SHA-256 allowlist. The 4 GB / Large Address Aware patch and changes to PE timestamps or checksums no longer disable rotation.

Before installing hooks, the DLL checks the engine's in-memory camera function, radar update, view-box pointer and input call sites. These checks target the supported Tiberium Wars 1.9 layout. A different engine layout still needs matching hook locations; identifying a file as Tiberium Wars does not make arbitrary addresses compatible. Unsupported layouts keep DirectInput forwarding and leave rotation disabled.

It loads from the base game and does not need to be added to each mod. Custom HUDs and other DLL wrappers may need additional support. Download checksums remain available for verifying release files; they do not control runtime activation.

## Build

On Windows, install [Zig 0.13.0](https://ziglang.org/download/0.13.0/) and run:

```powershell
.\build.ps1 -ZigPath C:\Tools\zig\zig.exe
.\test.ps1 -ZigPath C:\Tools\zig\zig.exe -SkipBuild
.\package.ps1 -ZigPath C:\Tools\zig\zig.exe -SkipBuild
```

The DLL is written to `build/dinput8.dll`; the release ZIP goes to `dist/`. The tests run outside the game, including a Direct3D 9 rendering check.

Project code is [MIT licensed](LICENSE). Vendored [MinHook 1.3.4](https://github.com/TsudaKageyu/minhook/tree/c3fcafdc10146beb5919319d0683e44e3c30d537) retains its BSD licenses in [THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt).
