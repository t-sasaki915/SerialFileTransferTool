#include <windows.h>

#define MAIN_WINDOW_CLASS_NAME L"SFTT_MAINWINDOW_CLASS"

LRESULT CALLBACK MainWindowWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcW(hwnd, wMsg, wParam, lParam);
}

int main(void)
{
    HINSTANCE mainInstance = GetModuleHandleW(NULL);

    WNDCLASSEXW mainWindowClass;
    ZeroMemory(&mainWindowClass, sizeof(mainWindowClass));
    mainWindowClass.cbSize = sizeof(mainWindowClass);
    mainWindowClass.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    mainWindowClass.hInstance = mainInstance;
    mainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    mainWindowClass.lpfnWndProc = MainWindowWndProc;

    RegisterClassExW(&mainWindowClass);

    HWND mainWindow = CreateWindowW(
        MAIN_WINDOW_CLASS_NAME,
        L"SerialFileTransferTool",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        mainInstance,
        NULL);

    ShowWindow(mainWindow, SW_SHOWNORMAL);
    UpdateWindow(mainWindow);

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessageW(&msg, mainWindow, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            break;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
