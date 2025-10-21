#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> g_running(true);

// 오버레이 윈도우 프로시저
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 화면 중앙 좌표
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        int cx = screenW / 2;
        int cy = screenH / 2;

        // 십자가 크기 (짧게 줄임)
        int size = 5; // 선 길이 5픽셀
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0)); // 빨간색 2px
        HGDIOBJ oldPen = SelectObject(hdc, hPen);

        // 가로선
        MoveToEx(hdc, cx - size, cy, NULL);
        LineTo(hdc, cx + size, cy);

        // 세로선
        MoveToEx(hdc, cx, cy - size, NULL);
        LineTo(hdc, cx, cy + size);

        SelectObject(hdc, oldPen);
        DeleteObject(hPen);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// 윈도우 클래스 등록
void RegisterOverlayWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"OverlayWindow";
    wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255)); // 투명색
    RegisterClass(&wc);
}

// 오버레이 윈도우 생성
HWND CreateOverlayWindow(HINSTANCE hInstance) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        L"OverlayWindow", NULL,
        WS_POPUP,
        0, 0, w, h,
        NULL, NULL, hInstance, NULL
    );

    // 마젠타 투명 처리
    SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return hwnd;
}

// 메시지 루프 (별도 스레드에서 실행)
void RunMessageLoop() {
    MSG msg;
    while (g_running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }
}

// 프로그램 진입점
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    RegisterOverlayWindow(hInstance);

    HWND hwndOverlay = nullptr;
    std::thread msgThread(RunMessageLoop);

    while (true) {
        std::cout << "\n=== 조준선 오버레이 메뉴 ===\n";
        std::cout << "1. 십자 표시\n";
        std::cout << "2. 조준선 제거 후 종료\n";
        std::cout << "선택: ";
        int choice;
        std::cin >> choice;

        if (choice == 1) {
            if (hwndOverlay) DestroyWindow(hwndOverlay);
            hwndOverlay = CreateOverlayWindow(hInstance);
            InvalidateRect(hwndOverlay, NULL, TRUE); // 십자가 그림
        }
        else if (choice == 2) {
            if (hwndOverlay) DestroyWindow(hwndOverlay);
            g_running = false;
            break;
        }
    }

    msgThread.join();
    return 0;
}
