# Minimap Rotator 1.0.3: fewer memory-readability checks

## What changed
- `source/camera_adapter.c`: `mr_camera_angle` no longer calls `readable()` (`VirtualQuery`) four times per frame. A new cache (`source/vcache.h`) keeps each positive result and reuses it for up to 30 calls (frames), then drops everything and validates again.
- Invalidation, so a stale result cannot lead to reading freed memory: the cached result is ignored at once if the pointer value changes or a larger size is requested; the whole cache is dropped every 30th call, on any rejected check or non-finite result, and at `mr_camera_bind`. Failures are never cached.
- Version 1.0.3 (VERSION, embedded build string, README, SHA256SUMS). `Minimap_Rotator_KW` has the same adapter code (only its addresses differ) and received the same change as 1.0.1.

## Not done: `compose()` reuse of the render target and state block
Skipped on purpose. The DLL hooks Present only, not `Reset`. A D3DPOOL_DEFAULT texture or state block held across frames would make the game's own `Reset` fail (D3DERR_INVALIDCALL) because default-pool resources must be released first, and there is no hook to release them. Doing it safely needs a Reset hook, which is a bigger change than this fix.

## How tested
- `tests/test_vcache.c` (added to `test.ps1`): first query validates, 28 frames reuse, the 30th revalidates, a changed pointer revalidates mid-interval, a larger size revalidates, slots are independent, failures are not cached, a pointer that turns unreadable fails on revalidation, reset drops everything.
- Full `test.ps1` passes (TW), and the KW `test.ps1` with the real `cnc3ep1.dat` passes, including the D3D9 rendering and device-reset test.

## Residual risk and what the tests cannot show
- Between validations the DLL still reads the pointers without a guarded read. If the game frees the display, camera manager or camera array within the same 30-frame window without the pointer value changing, the read stays unguarded for up to 30 frames. At 30 fps that is about one second. The previous per-frame `VirtualQuery` only proved the page was mapped and did not protect against freed-but-mapped memory either. The project has no SEH-style safe read available in this toolchain (Zig/clang x86), so the reads are not wrapped.
- No live game run. The fps effect is inferred from the engine profile (about 20 to 22% of the main thread), not measured. The owner measures it with the TWEngine profiler kit.

## Owner test steps
1. Play a 1v1 for a few minutes with Alt rotation; confirm the minimap rotates as before.
2. Start and end a match, load a save, alt-tab out and back; confirm no crash.
3. Compare a TWEngine profile with the 1.0.2 capture (rollback DLL is in `release/rollback`).

## Test note: pre-existing flake
KW `test_input_abi` fails intermittently (about 40% of runs) on unmodified HEAD code as well; it does not compile the changed files. Reruns pass. Not caused by this change; not fixed here.

## Deployed (2026-11-13)
- TW 1.0.3 DLL `fa788a8d...3902` copied to `RetailExe\1.9\dinput8.dll` and to the game root `dinput8.dll` (hashes verified equal to the build).
- KW 1.0.1 DLL `0a2b209b...51ab` copied to `Kane's Wrath\RetailExe\1.2\dinput8.dll` (verified).
- Rollback: `Minimap_Rotator\release\rollback\dinput8-1.0.2.dll` (`a0ce0330...0bcb56`) for both TW locations, and `Minimap_Rotator_KW\dist\rollback\dinput8-1.0.0.dll` (`f30f99a1...5135`) for KW: close the game and copy them over the installed `dinput8.dll`.
- GitHub release v1.0.3 is not published yet: it follows the owner's play-test (standing rule).
