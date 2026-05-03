#include <wchar.h>
#include <windows.h>
#include <winuser.h>

#define COM_PORT_TRY_MAX 20
#define MAX_PATH_LENGTH 260

#define CANNOT_OPEN_FILE_ERROR_TITLE L"SFTT"
#define CANNOT_OPEN_FILE_ERROR_MSG L"Cannot open the file '%ls': %lu."
#define CANNOT_OPEN_FILE_ERROR_MSG_LENGTH 350

#define CANNOT_GET_FILE_SIZE_ERROR_TITLE L"SFTT"
#define CANNOT_GET_FILE_SIZE_ERROR_MSG L"Cannot get the size of the file '%ls': %lu."
#define CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH 400

#define PLEASE_SPECIFY_PORT_ERROR_TITLE L"SFTT"
#define PLEASE_SPECIFY_PORT_ERROR_MSG L"Please specify the port."

#define CANNOT_OPEN_COM_PORT_ERROR_TITLE L"SFTT"
#define CANNOT_OPEN_COM_PORT_ERROR_MSG L"Cannot open the COM port '%ls': %lu."
#define CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH 100

#define MAIN_WINDOW_CLASS_NAME L"SFTT_MAINWINDOW_CLASS"
#define MAIN_WINDOW_TITLE_SEND_MODE L"SFTT - Send"
#define MAIN_WINDOW_TITLE_RECEIVE_MODE L"SFTT - Receive"
#define MAIN_WINDOW_WIDTH 274
#define MAIN_WINDOW_HEIGHT_SEND_MODE 177
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

#define FILE_TO_SEND_LABEL_TEXT L"File: "
#define FILE_TO_SEND_LABEL_TEXT_LENGTH 6
#define FILE_TO_SEND_LABEL_X 7
#define FILE_TO_SEND_LABEL_Y 79

#define SEND_FILE_PATH_TEXTBOX_X 35
#define SEND_FILE_PATH_TEXTBOX_Y 76
#define SEND_FILE_PATH_TEXTBOX_WIDTH 148
#define SEND_FILE_PATH_TEXTBOX_HEIGHT 20

#define SEND_FILE_PATH_BROWSE_BUTTON_LABEL L"Browse"
#define SEND_FILE_PATH_BROWSE_BUTTON_X 190
#define SEND_FILE_PATH_BROWSE_BUTTON_Y 76
#define SEND_FILE_PATH_BROWSE_BUTTON_WIDTH 70
#define SEND_FILE_PATH_BROWSE_BUTTON_HEIGHT 20
#define SEND_FILE_PATH_BROWSE_BUTTON_ID 104

#define SEND_FILE_BUTTON_LABEL L"Send a file"
#define SEND_FILE_BUTTON_X 7
#define SEND_FILE_BUTTON_Y 103
#define SEND_FILE_BUTTON_WIDTH 253
#define SEND_FILE_BUTTON_HEIGHT 40
#define SEND_FILE_BUTTON_ID 105

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
HWND SEND_FILE_PATH_TEXTBOX;
HWND SEND_FILE_PATH_BROWSE_BUTTON;
HWND SEND_FILE_BUTTON;

void FreePortListItemDataPointers(void)
{
    int itemCount = (int)SendMessageW(PORT_SELECT_COMBO_BOX, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

    for (int i = 0; i < itemCount; i++)
    {
        LRESULT itemData = SendMessageW(PORT_SELECT_COMBO_BOX, CB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
        if (itemData != CB_ERR && itemData != 0)
        {
            free((wchar_t *)itemData);
        }
    }

    SendMessageW(PORT_SELECT_COMBO_BOX, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
}

void UpdatePortList(void)
{
    EnableWindow(PORT_SELECT_COMBO_BOX, FALSE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, FALSE);

    FreePortListItemDataPointers();

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

        wchar_t *persistentPortName = _wcsdup(portName);
        int index = (int)SendMessageW(PORT_SELECT_COMBO_BOX, CB_ADDSTRING, (WPARAM)0, (LPARAM)friendlyPortName);
        SendMessageW(PORT_SELECT_COMBO_BOX, CB_SETITEMDATA, (WPARAM)index, (LPARAM)persistentPortName);

        CloseHandle(hComm);
    }

    EnableWindow(PORT_SELECT_COMBO_BOX, TRUE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, TRUE);
}

void StartReceiving(void)
{
    SetWindowTextW(START_RECEIVING_BUTTON, START_RECEIVING_BUTTON_LABEL_STOP);

    EnableWindow(PORT_SELECT_COMBO_BOX, FALSE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, FALSE);
    EnableWindow(MODE_CHANGE_BUTTON_SEND_MODE, FALSE);

    IS_RECEIVING = TRUE;
}

void StopReceiving(void)
{
    SetWindowTextW(START_RECEIVING_BUTTON, START_RECEIVING_BUTTON_LABEL_START);

    EnableWindow(PORT_SELECT_COMBO_BOX, TRUE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, TRUE);
    EnableWindow(MODE_CHANGE_BUTTON_SEND_MODE, TRUE);

    IS_RECEIVING = FALSE;
}

void SetApplicationMode(ApplicationMode appMode)
{
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

    int sendModeComponentShowMode = (appMode == APPLICATION_MODE_SEND_MODE) ? SW_SHOW : SW_HIDE;
    ShowWindow(SEND_FILE_PATH_TEXTBOX, sendModeComponentShowMode);
    ShowWindow(SEND_FILE_PATH_BROWSE_BUTTON, sendModeComponentShowMode);
    ShowWindow(SEND_FILE_BUTTON, sendModeComponentShowMode);

    InvalidateRect(MAIN_WINDOW, NULL, FALSE);
}

void SendFile(void)
{
    wchar_t filePathToSend[MAX_PATH_LENGTH];
    GetWindowTextW(SEND_FILE_PATH_TEXTBOX, filePathToSend, MAX_PATH_LENGTH);

    HANDLE hFileToSend =
        CreateFileW(filePathToSend, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFileToSend == INVALID_HANDLE_VALUE)
    {
        wchar_t cannotOpenFileMsg[CANNOT_OPEN_FILE_ERROR_MSG_LENGTH];
        swprintf(
            cannotOpenFileMsg,
            CANNOT_OPEN_FILE_ERROR_MSG_LENGTH,
            CANNOT_OPEN_FILE_ERROR_MSG,
            filePathToSend,
            GetLastError());

        MessageBoxW(MAIN_WINDOW, cannotOpenFileMsg, CANNOT_OPEN_FILE_ERROR_TITLE, MB_ICONERROR | MB_OK);

        return;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFileToSend, &fileSize))
    {
        wchar_t cannotGetFileSizeMsg[CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH];
        swprintf(
            cannotGetFileSizeMsg,
            CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH,
            CANNOT_GET_FILE_SIZE_ERROR_MSG,
            filePathToSend,
            GetLastError());

        MessageBoxW(MAIN_WINDOW, cannotGetFileSizeMsg, CANNOT_GET_FILE_SIZE_ERROR_TITLE, MB_ICONERROR | MB_OK);

        CloseHandle(hFileToSend);

        return;
    }

    int portListIndex = (int)SendMessageW(PORT_SELECT_COMBO_BOX, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    LRESULT portListData = SendMessageW(PORT_SELECT_COMBO_BOX, CB_GETITEMDATA, (WPARAM)portListIndex, (LPARAM)0);
    if (portListData == CB_ERR || portListData == 0)
    {
        MessageBoxW(
            MAIN_WINDOW,
            PLEASE_SPECIFY_PORT_ERROR_MSG,
            PLEASE_SPECIFY_PORT_ERROR_TITLE,
            MB_ICONINFORMATION | MB_OK);

        CloseHandle(hFileToSend);

        return;
    }
    wchar_t *portName = (wchar_t *)portListData;

    HANDLE hComPort = CreateFileW(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hComPort == INVALID_HANDLE_VALUE)
    {
        wchar_t cannotOpenComPortMsg[CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH];
        swprintf(
            cannotOpenComPortMsg,
            CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH,
            CANNOT_OPEN_COM_PORT_ERROR_MSG,
            portName,
            GetLastError());

        MessageBoxW(MAIN_WINDOW, cannotOpenComPortMsg, CANNOT_OPEN_COM_PORT_ERROR_TITLE, MB_ICONERROR | MB_OK);

        CloseHandle(hFileToSend);

        return;
    }

    CloseHandle(hComPort);
    CloseHandle(hFileToSend);
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

            if (CURRENT_APPLICATION_MODE == APPLICATION_MODE_SEND_MODE)
            {
                TextOutW(
                    hdc,
                    FILE_TO_SEND_LABEL_X,
                    FILE_TO_SEND_LABEL_Y,
                    FILE_TO_SEND_LABEL_TEXT,
                    FILE_TO_SEND_LABEL_TEXT_LENGTH);
            }

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
                case SEND_FILE_PATH_BROWSE_BUTTON_ID: {
                    wchar_t filePath[MAX_PATH_LENGTH] = L"";

                    OPENFILENAMEW ofn;
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = MAIN_WINDOW;
                    ofn.lpstrFile = filePath;
                    ofn.nMaxFile = MAX_PATH_LENGTH;
                    ofn.lpstrFilter = L"All Files\0*.*\0";
                    ofn.nFilterIndex = 1;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

                    if (GetOpenFileNameW(&ofn))
                    {
                        SetWindowTextW(SEND_FILE_PATH_TEXTBOX, filePath);
                    }

                    return 0;
                }
                case SEND_FILE_BUTTON_ID: {
                    EnableWindow(PORT_SELECT_COMBO_BOX, FALSE);
                    EnableWindow(PORT_SELECT_UPDATE_BUTTON, FALSE);
                    EnableWindow(MODE_CHANGE_BUTTON_RECEIVE_MODE, FALSE);
                    EnableWindow(SEND_FILE_PATH_TEXTBOX, FALSE);
                    EnableWindow(SEND_FILE_PATH_BROWSE_BUTTON, FALSE);
                    EnableWindow(SEND_FILE_BUTTON, FALSE);

                    SendFile();

                    EnableWindow(PORT_SELECT_COMBO_BOX, TRUE);
                    EnableWindow(PORT_SELECT_UPDATE_BUTTON, TRUE);
                    EnableWindow(MODE_CHANGE_BUTTON_RECEIVE_MODE, TRUE);
                    EnableWindow(SEND_FILE_PATH_TEXTBOX, TRUE);
                    EnableWindow(SEND_FILE_PATH_BROWSE_BUTTON, TRUE);
                    EnableWindow(SEND_FILE_BUTTON, TRUE);

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

    SEND_FILE_PATH_TEXTBOX = CreateWindowW(
        L"EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        SEND_FILE_PATH_TEXTBOX_X,
        SEND_FILE_PATH_TEXTBOX_Y,
        SEND_FILE_PATH_TEXTBOX_WIDTH,
        SEND_FILE_PATH_TEXTBOX_HEIGHT,
        MAIN_WINDOW,
        NULL,
        mainInstance,
        NULL);

    SendMessageW(SEND_FILE_PATH_TEXTBOX, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    SEND_FILE_PATH_BROWSE_BUTTON = CreateWindowW(
        L"BUTTON",
        SEND_FILE_PATH_BROWSE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        SEND_FILE_PATH_BROWSE_BUTTON_X,
        SEND_FILE_PATH_BROWSE_BUTTON_Y,
        SEND_FILE_PATH_BROWSE_BUTTON_WIDTH,
        SEND_FILE_PATH_BROWSE_BUTTON_HEIGHT,
        MAIN_WINDOW,
        (HMENU)SEND_FILE_PATH_BROWSE_BUTTON_ID,
        mainInstance,
        NULL);

    SendMessageW(SEND_FILE_PATH_BROWSE_BUTTON, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    SEND_FILE_BUTTON = CreateWindowW(
        L"BUTTON",
        SEND_FILE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        SEND_FILE_BUTTON_X,
        SEND_FILE_BUTTON_Y,
        SEND_FILE_BUTTON_WIDTH,
        SEND_FILE_BUTTON_HEIGHT,
        MAIN_WINDOW,
        (HMENU)SEND_FILE_BUTTON_ID,
        mainInstance,
        NULL);

    SendMessageW(SEND_FILE_BUTTON, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

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

    FreePortListItemDataPointers();

    if (IS_RECEIVING)
    {
        StopReceiving();
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
