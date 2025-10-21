#include <Windows.h>
#include <d3d9.h>
#include "MinHook.h"

typedef HRESULT(APIENTRY* EndScene_t)(LPDIRECT3DDEVICE9);
EndScene_t oEndScene = nullptr;

HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
    D3DRECT horizontal = { 960 - 10, 540, 960 + 10, 541 };
    D3DRECT vertical = { 960, 540 - 10, 961, 540 + 10 };
    pDevice->Clear(1, &horizontal, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 0, 0);
    pDevice->Clear(1, &vertical, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 0, 0);
    return oEndScene(pDevice);
}

DWORD WINAPI InitHook(LPVOID) {
    if (MH_Initialize() != MH_OK) return 1;

    HWND hWnd = CreateWindowA("STATIC", "Dummy", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, nullptr, nullptr);
    LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;

    LPDIRECT3DDEVICE9 pDevice = nullptr;
    pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);

    void** vTable = *reinterpret_cast<void***>(pDevice);
    void* target = vTable[42];

    MH_CreateHook(target, &hkEndScene, reinterpret_cast<void**>(&oEndScene));
    MH_EnableHook(target);

    pDevice->Release();
    pD3D->Release();
    DestroyWindow(hWnd);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, InitHook, nullptr, 0, nullptr);
    }
    return TRUE;
}
