#include <wchar.h>
#include <windows.h>

#define COM_PORT_TRY_MAX 20

#define MAIN_WINDOW_CLASS_NAME L"SFTT_MAINWINDOW_CLASS"
#define MAIN_WINDOW_TITLE_SEND_MODE L"SFTT - Send"
#define MAIN_WINDOW_TITLE_RECEIVE_MODE L"SFTT - Receive"
#define MAIN_WINDOW_WIDTH 274
#define MAIN_WINDOW_HEIGHT_SEND_MODE 400
#define MAIN_WINDOW_HEIGHT_RECEIVE_MODE 150

#define UI_FONT_NAME L"MS Shell Dlg"
#define UI_FONT_SIZE 15

#define PORT_SELECT_LABEL_TEXT L"Port: "
#define PORT_SELECT_LABEL_TEXT_LENGTH 6
#define PORT_SELECT_LABEL_X 7
#define PORT_SELECT_LABEL_Y 7

#define PORT_SELECT_COMBOBOX_X 35
#define PORT_SELECT_COMBOBOX_Y 4
#define PORT_SELECT_COMBOBOX_WIDTH 150
#define PORT_SELECT_COMBOBOX_HEIGHT 200
#define PORT_SELECT_COMBOBOX_ID 1000

#define PORT_SELECT_UPDATE_BUTTON_LABEL L"Update"
#define PORT_SELECT_UPDATE_BUTTON_X 190
#define PORT_SELECT_UPDATE_BUTTON_Y 4
#define PORT_SELECT_UPDATE_BUTTON_WIDTH 70
#define PORT_SELECT_UPDATE_BUTTON_HEIGHT 20
#define PORT_SELECT_UPDATE_BUTTON_ID 100

#define MODE_CHANGE_BUTTON_SEND_MODE_LABEL L"Send Mode"
#define MODE_CHANGE_BUTTON_SEND_MODE_X 7
#define MODE_CHANGE_BUTTON_SEND_MODE_Y 30
#define MODE_CHANGE_BUTTON_SEND_MODE_WIDTH 123
#define MODE_CHANGE_BUTTON_SEND_MODE_HEIGHT 40
#define MODE_CHANGE_BUTTON_SEND_MODE_BUTTON_ID 101

#define MODE_CHANGE_BUTTON_RECEIVE_MODE_LABEL L"Receive Mode"
#define MODE_CHANGE_BUTTON_RECEIVE_MODE_X 137
#define MODE_CHANGE_BUTTON_RECEIVE_MODE_Y 30
#define MODE_CHANGE_BUTTON_RECEIVE_MODE_WIDTH 123
#define MODE_CHANGE_BUTTON_RECEIVE_MODE_HEIGHT 40
#define MODE_CHANGE_BUTTON_RECEIVE_MODE_BUTTON_ID 102

#define START_RECEIVING_BUTTON_LABEL_START L"Start Receiving"
#define START_RECEIVING_BUTTON_LABEL_STOP L"Stop Receiving"
#define START_RECEIVING_BUTTON_X 7
#define START_RECEIVING_BUTTON_Y 76
#define START_RECEIVING_BUTTON_WIDTH 253
#define START_RECEIVING_BUTTON_HEIGHT 40
#define START_RECEIVING_BUTTON_ID 103

typedef enum
{
    APPLICATION_MODE_SEND_MODE,
    APPLICATION_MODE_RECEIVE_MODE
} ApplicationMode;

ApplicationMode CURRENT_APPLICATION_MODE;
BOOL IS_RECEIVING = FALSE;

HFONT UI_FONT;

HWND MAIN_WINDOW;
HWND PORT_SELECT_COMBO_BOX;
HWND PORT_SELECT_UPDATE_BUTTON;
HWND MODE_CHANGE_BUTTON_SEND_MODE;
HWND MODE_CHANGE_BUTTON_RECEIVE_MODE;
HWND START_RECEIVING_BUTTON;

void UpdatePortList(void)
{
    SendMessageW(PORT_SELECT_COMBO_BOX, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

    for (int i = 1; i <= COM_PORT_TRY_MAX; i++)
    {
        wchar_t friendlyPortName[10];
        swprintf(friendlyPortName, 10, L"COM%d", i);
        wchar_t portName[20];
        swprintf(portName, 20, L"\\\\.\\%ls", friendlyPortName);

        HANDLE hComm = CreateFileW(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (hComm == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            continue;
        }

        SendMessageW(PORT_SELECT_COMBO_BOX, CB_ADDSTRING, (WPARAM)0, (LPARAM)friendlyPortName);

        CloseHandle(hComm);
    }
    SendMessageW(PORT_SELECT_COMBO_BOX, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

void StartReceiving(void)
{
    SetWindowTextW(START_RECEIVING_BUTTON, START_RECEIVING_BUTTON_LABEL_STOP);

    EnableWindow(PORT_SELECT_COMBO_BOX, FALSE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, FALSE);

    IS_RECEIVING = TRUE;
}

void StopReceiving(void)
{
    SetWindowTextW(START_RECEIVING_BUTTON, START_RECEIVING_BUTTON_LABEL_START);

    EnableWindow(PORT_SELECT_COMBO_BOX, TRUE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, TRUE);

    IS_RECEIVING = FALSE;
}

void SetApplicationMode(ApplicationMode appMode)
{
    if (appMode != APPLICATION_MODE_RECEIVE_MODE && IS_RECEIVING)
    {
        StopReceiving();
    }

    CURRENT_APPLICATION_MODE = appMode;

    LPCWSTR mainWindowTitle;
    int mainWindowHeight;
    switch (appMode)
    {
        case APPLICATION_MODE_SEND_MODE: {
            mainWindowTitle = MAIN_WINDOW_TITLE_SEND_MODE;
            mainWindowHeight = MAIN_WINDOW_HEIGHT_SEND_MODE;
            break;
        }
        case APPLICATION_MODE_RECEIVE_MODE: {
            mainWindowTitle = MAIN_WINDOW_TITLE_RECEIVE_MODE;
            mainWindowHeight = MAIN_WINDOW_HEIGHT_RECEIVE_MODE;
            break;
        }
    }

    SendMessageW(MAIN_WINDOW, WM_SETTEXT, (WPARAM)0, (LPARAM)mainWindowTitle);
    SetWindowPos(
        MAIN_WINDOW,
        NULL,
        0,
        0,
        MAIN_WINDOW_WIDTH,
        mainWindowHeight,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

    EnableWindow(MODE_CHANGE_BUTTON_SEND_MODE, appMode != APPLICATION_MODE_SEND_MODE);
    EnableWindow(MODE_CHANGE_BUTTON_RECEIVE_MODE, appMode != APPLICATION_MODE_RECEIVE_MODE);

    int startReceivingButtonShowMode = (appMode == APPLICATION_MODE_RECEIVE_MODE) ? SW_SHOW : SW_HIDE;
    ShowWindow(START_RECEIVING_BUTTON, startReceivingButtonShowMode);
}

LRESULT CALLBACK MainWindowWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_DESTROY: {
            PostQuitMessage(0);

            return 0;
        }
        case WM_PAINT: {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hwnd, &ps);

            HFONT oldFont = SelectObject(hdc, UI_FONT);

            TextOutW(
                hdc,
                PORT_SELECT_LABEL_X,
                PORT_SELECT_LABEL_Y,
                PORT_SELECT_LABEL_TEXT,
                PORT_SELECT_LABEL_TEXT_LENGTH);

            SelectObject(hdc, oldFont);

            EndPaint(hwnd, &ps);

            return 0;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hwnd, &rect);

            HBRUSH bkgndBrush = GetSysColorBrush(COLOR_WINDOW);
            FillRect(hdc, &rect, bkgndBrush);

            return 1;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam))
            {
                case PORT_SELECT_UPDATE_BUTTON_ID: {
                    UpdatePortList();

                    return 0;
                }
                case MODE_CHANGE_BUTTON_SEND_MODE_BUTTON_ID: {
                    SetApplicationMode(APPLICATION_MODE_SEND_MODE);

                    return 0;
                }
                case MODE_CHANGE_BUTTON_RECEIVE_MODE_BUTTON_ID: {
                    SetApplicationMode(APPLICATION_MODE_RECEIVE_MODE);

                    return 0;
                }
                case START_RECEIVING_BUTTON_ID: {
                    if (IS_RECEIVING)
                    {
                        StopReceiving();
                    }
                    else
                    {
                        StartReceiving();
                    }

                    return 0;
                }
                default: {
                    return DefWindowProcW(hwnd, wMsg, wParam, lParam);
                }
            }
        }
        default: {
            return DefWindowProcW(hwnd, wMsg, wParam, lParam);
        }
    }
}

int main(void)
{
    HINSTANCE mainInstance = GetModuleHandleW(NULL);

    HICON mainWindowIcon;
    SHSTOCKICONINFO mainWindowIconInfo;
    mainWindowIconInfo.cbSize = sizeof(mainWindowIconInfo);
    SHGetStockIconInfo(SIID_NETWORKCONNECT, SHGSI_ICON, &mainWindowIconInfo);
    mainWindowIcon = mainWindowIconInfo.hIcon;

    WNDCLASSEXW mainWindowClass;
    ZeroMemory(&mainWindowClass, sizeof(mainWindowClass));
    mainWindowClass.cbSize = sizeof(mainWindowClass);
    mainWindowClass.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    mainWindowClass.hInstance = mainInstance;
    mainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    mainWindowClass.hIcon = mainWindowIcon;
    mainWindowClass.lpfnWndProc = MainWindowWndProc;

    RegisterClassExW(&mainWindowClass);

    UI_FONT = CreateFontW(
        UI_FONT_SIZE,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        UI_FONT_NAME);

    MAIN_WINDOW = CreateWindowW(
        MAIN_WINDOW_CLASS_NAME,
        NULL,
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        MAIN_WINDOW_WIDTH,
        0,
        NULL,
        NULL,
        mainInstance,
        NULL);

    PORT_SELECT_COMBO_BOX = CreateWindowW(
        L"COMBOBOX",
        NULL,
        CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL | WS_VISIBLE,
        PORT_SELECT_COMBOBOX_X,
        PORT_SELECT_COMBOBOX_Y,
        PORT_SELECT_COMBOBOX_WIDTH,
        PORT_SELECT_COMBOBOX_HEIGHT,
        MAIN_WINDOW,
        (HMENU)PORT_SELECT_COMBOBOX_ID,
        mainInstance,
        NULL);

    SendMessageW(PORT_SELECT_COMBO_BOX, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    PORT_SELECT_UPDATE_BUTTON = CreateWindowW(
        L"BUTTON",
        PORT_SELECT_UPDATE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        PORT_SELECT_UPDATE_BUTTON_X,
        PORT_SELECT_UPDATE_BUTTON_Y,
        PORT_SELECT_UPDATE_BUTTON_WIDTH,
        PORT_SELECT_UPDATE_BUTTON_HEIGHT,
        MAIN_WINDOW,
        (HMENU)PORT_SELECT_UPDATE_BUTTON_ID,
        mainInstance,
        NULL);

    SendMessageW(PORT_SELECT_UPDATE_BUTTON, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    MODE_CHANGE_BUTTON_SEND_MODE = CreateWindowW(
        L"BUTTON",
        MODE_CHANGE_BUTTON_SEND_MODE_LABEL,
        WS_CHILD | WS_VISIBLE,
        MODE_CHANGE_BUTTON_SEND_MODE_X,
        MODE_CHANGE_BUTTON_SEND_MODE_Y,
        MODE_CHANGE_BUTTON_SEND_MODE_WIDTH,
        MODE_CHANGE_BUTTON_SEND_MODE_HEIGHT,
        MAIN_WINDOW,
        (HMENU)MODE_CHANGE_BUTTON_SEND_MODE_BUTTON_ID,
        mainInstance,
        NULL);

    SendMessageW(MODE_CHANGE_BUTTON_SEND_MODE, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    MODE_CHANGE_BUTTON_RECEIVE_MODE = CreateWindowW(
        L"BUTTON",
        MODE_CHANGE_BUTTON_RECEIVE_MODE_LABEL,
        WS_CHILD | WS_VISIBLE,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_X,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_Y,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_WIDTH,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_HEIGHT,
        MAIN_WINDOW,
        (HMENU)MODE_CHANGE_BUTTON_RECEIVE_MODE_BUTTON_ID,
        mainInstance,
        NULL);

    SendMessageW(MODE_CHANGE_BUTTON_RECEIVE_MODE, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    START_RECEIVING_BUTTON = CreateWindowW(
        L"BUTTON",
        START_RECEIVING_BUTTON_LABEL_START,
        WS_CHILD | WS_VISIBLE,
        START_RECEIVING_BUTTON_X,
        START_RECEIVING_BUTTON_Y,
        START_RECEIVING_BUTTON_WIDTH,
        START_RECEIVING_BUTTON_HEIGHT,
        MAIN_WINDOW,
        (HMENU)START_RECEIVING_BUTTON_ID,
        mainInstance,
        NULL);

    SendMessageW(START_RECEIVING_BUTTON, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    UpdatePortList();
    SetApplicationMode(APPLICATION_MODE_SEND_MODE);

    ShowWindow(MAIN_WINDOW, SW_SHOWNORMAL);
    UpdateWindow(MAIN_WINDOW);

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            break;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (mainWindowIcon != NULL)
    {
        DestroyIcon(mainWindowIcon);
    }
    if (UI_FONT != NULL)
    {
        DeleteObject(UI_FONT);
    }

    return 0;
}
