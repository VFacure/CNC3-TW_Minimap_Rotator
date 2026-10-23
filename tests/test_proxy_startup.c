#include <windows.h>
#include <unknwn.h>
#include <stdio.h>
#include "../tools/minhook/include/MinHook.h"

typedef HRESULT (WINAPI *CreateInput)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI DirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
static CreateInput original_create;
static unsigned initialize_calls, overlay_entries, overlay_returns;

/* Replace the game initializer only in this standalone test executable. */
DWORD WINAPI MR_Initialize(void *reserved) {
    (void)reserved;
    initialize_calls++;
    return 0;
}

static HRESULT WINAPI overlay_create(HINSTANCE instance, DWORD version, REFIID iid,
                                    LPVOID *out, LPUNKNOWN outer) {
    overlay_entries++;
    HRESULT result = original_create(instance, version, iid, out, outer);
    overlay_returns++;
    return result;
}

#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)

int main(void) {
    const IID iid = {0xbf798030, 0x483a, 0x4da2, {0xaa, 0x99, 0x5d, 0x64, 0xed, 0x36, 0x97, 0x00}};
    CHECK(MH_Initialize() == MH_OK);
    CHECK(MH_CreateHook((void *)&DirectInput8Create, (void *)&overlay_create,
                        (void **)&original_create) == MH_OK);
    CHECK(MH_EnableHook((void *)&DirectInput8Create) == MH_OK);
    CreateInput volatile entry = &DirectInput8Create;
    void *input = NULL;
    HRESULT hr = entry(GetModuleHandleW(NULL), 0x0800, &iid, &input, NULL);
    CHECK(SUCCEEDED(hr) && input != NULL);
    ((IUnknown *)input)->lpVtbl->Release((IUnknown *)input);
    CHECK(overlay_entries == 1 && overlay_returns == 1);
    CHECK(initialize_calls == 1);
    CHECK(MH_DisableHook((void *)&DirectInput8Create) == MH_OK);
    input = NULL;
    hr = entry(GetModuleHandleW(NULL), 0x0800, &iid, &input, NULL);
    CHECK(SUCCEEDED(hr) && input != NULL);
    ((IUnknown *)input)->lpVtbl->Release((IUnknown *)input);
    CHECK(initialize_calls == 2);
    CHECK(MH_Uninitialize() == MH_OK);
    puts("PASS: initialization reached through an overlay trampoline and a direct call; real DirectInput forwarding preserved. No game used.");
    return 0;
}
