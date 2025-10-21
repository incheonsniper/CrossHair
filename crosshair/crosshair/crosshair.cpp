#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// GDI+ 초기화/종료
ULONG_PTR gdiplusToken;
void InitGDIPlus() {
    GdiplusStartupInput input;
    GdiplusStartup(&gdiplusToken, &input, NULL);
}
void ShutdownGDIPlus() {
    GdiplusShutdown(gdiplusToken);
}

// 오버레이 타입
enum class OverlayType { None, Cross };
OverlayType g_overlayType = OverlayType::None;

// 오버레이 윈도우 프로시저
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Graphics graphics(hdc);
        graphics.Clear(Color(1, 1, 1)); // 투명 배경

        if (g_overlayType == OverlayType::Cross) {
            Pen pen(Color(255, 255, 0, 0), 2); // 빨간색, 두께 2
            int cx = 10, cy = 10;
            graphics.DrawLine(&pen, cx - 5, cy, cx + 5, cy); // 수평선
            graphics.DrawLine(&pen, cx, cy - 5, cx, cy + 5); // 수직선
        }

        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// 오버레이 윈도우 클래스 등록
void RegisterOverlayWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"OverlayWindow";
    wc.hbrBackground = NULL;
    RegisterClass(&wc);
}

// 오버레이 윈도우 생성
HWND CreateOverlayWindow(HINSTANCE hInstance, int width, int height) {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        L"OverlayWindow", NULL,
        WS_POPUP,
        x, y, width, height,
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, RGB(1, 1, 1), 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

// 🧱 오버레이 클래스
class BaseOverlay {
public:
    virtual void Show(HWND hwnd) = 0;
    virtual ~BaseOverlay() {}
};

// ➕ 십자가 오버레이
class CrossOverlay : public BaseOverlay {
public:
    void Show(HWND hwnd) override {
        g_overlayType = OverlayType::Cross;
        InvalidateRect(hwnd, NULL, TRUE);
    }
};

// 콘솔 메뉴
void RunConsoleMenu(HINSTANCE hInstance) {
    HWND hwndOverlay = NULL;
    BaseOverlay* overlay = nullptr;

    while (true) {
        std::cout << "\n=== 십자가 오버레이 메뉴 ===\n";
        std::cout << "1. 십자가 표시\n";
        std::cout << "2. 오버레이 제거\n";
        std::cout << "선택: ";
        int choice;
        std::cin >> choice;

        if (hwndOverlay) {
            DestroyWindow(hwndOverlay);
            hwndOverlay = NULL;
            delete overlay;
            overlay = nullptr;
            g_overlayType = OverlayType::None;
        }

        if (choice == 1) {
            hwndOverlay = CreateOverlayWindow(hInstance, 20, 20);
            overlay = new CrossOverlay();
            overlay->Show(hwndOverlay);
        }
        else if (choice == 2) {
            std::cout << "오버레이 제거됨.\n";
        }

        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

// WinMain 진입점
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* fp;
    fp = freopen("CONIN$", "r", stdin);
    fp = freopen("CONOUT$", "w", stdout);
    fp = freopen("CONOUT$", "w", stderr);

    InitGDIPlus();
    RegisterOverlayWindow(hInstance);
    RunConsoleMenu(hInstance);
    ShutdownGDIPlus();
    return 0;
}

// yywrap 오류 방지용
extern "C" int yywrap() {
    return 1;
}
