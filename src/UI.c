#include <windows.h>

#include "Serial.h"
#include "UI.h"
#include "Util.h"

#define WM_SFTT_SHOW_ERROR_DIALOG WM_USER + 1

#define UI_FONT_NAME L"Tahoma"
#define UI_FONT_SIZE 15

#define MAIN_WINDOW_CLASS_NAME L"SFTT_MAINWINDOW_CLASS"
#define MAIN_WINDOW_TITLE_SEND_MODE L"SFTT - Send"
#define MAIN_WINDOW_TITLE_RECEIVE_MODE L"SFTT - Receive"
#define MAIN_WINDOW_WIDTH 274
#define MAIN_WINDOW_HEIGHT_SEND_MODE 177
#define MAIN_WINDOW_HEIGHT_RECEIVE_MODE 150

#define PORT_SELECT_LABEL_TEXT L"Port: "
#define PORT_SELECT_LABEL_TEXT_LENGTH 6
#define PORT_SELECT_LABEL_X 5
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
#define FILE_TO_SEND_LABEL_X 8
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

#define SEND_FILE_BUTTON_LABEL L"Send a File"
#define SEND_FILE_BUTTON_X 7
#define SEND_FILE_BUTTON_Y 103
#define SEND_FILE_BUTTON_WIDTH 253
#define SEND_FILE_BUTTON_HEIGHT 40
#define SEND_FILE_BUTTON_ID 105

HINSTANCE MAIN_INSTANCE;

LogicSet MAIN_LOGIC_SET;

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

ApplicationMode CURRENT_APPLICATION_MODE;

void InitialiseUI(LogicSet mainLogicSet)
{
    MAIN_INSTANCE = GetModuleHandleW(NULL);

    MAIN_LOGIC_SET = mainLogicSet;

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
        5, // CLEARTYPE_QUALITY (for Windows 2000 compatibility)
        DEFAULT_PITCH | FF_DONTCARE,
        UI_FONT_NAME);
}

void FreePortSelectListPortNamePointers(void)
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

void UpdatePortSelectList(void)
{
    EnableWindow(PORT_SELECT_COMBO_BOX, FALSE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, FALSE);

    FreePortSelectListPortNamePointers();

    AvailablePort availablePorts[COM_PORT_TRY_MAX];
    int numberOfAvailablePorts;
    GetAvailablePorts(availablePorts, &numberOfAvailablePorts);

    for (int i = 0; i < numberOfAvailablePorts; i++)
    {
        AvailablePort port = availablePorts[i];

        int itemIndex =
            (int)SendMessageW(PORT_SELECT_COMBO_BOX, CB_ADDSTRING, (WPARAM)0, (LPARAM)port.friendlyPortName);
        SendMessageW(PORT_SELECT_COMBO_BOX, CB_SETITEMDATA, (WPARAM)itemIndex, (LPARAM)port.portName);

        free(port.friendlyPortName);
    }

    EnableWindow(PORT_SELECT_COMBO_BOX, TRUE);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, TRUE);
}

void SetApplicationMode(ApplicationMode appMode)
{
    CURRENT_APPLICATION_MODE = appMode;

    LPCWSTR newMainWindowTitle = L"";
    int newMainWindowHeight = 0;
    switch (appMode)
    {
        case APPLICATION_MODE_SEND_MODE: {
            newMainWindowTitle = MAIN_WINDOW_TITLE_SEND_MODE;
            newMainWindowHeight = MAIN_WINDOW_HEIGHT_SEND_MODE;

            break;
        }
        case APPLICATION_MODE_RECEIVE_MODE: {
            newMainWindowTitle = MAIN_WINDOW_TITLE_RECEIVE_MODE;
            newMainWindowHeight = MAIN_WINDOW_HEIGHT_RECEIVE_MODE;

            break;
        }
    }

    SendMessageW(MAIN_WINDOW, WM_SETTEXT, (WPARAM)0, (LPARAM)newMainWindowTitle);
    SetWindowPos(
        MAIN_WINDOW,
        NULL,
        0,
        0,
        MAIN_WINDOW_WIDTH,
        newMainWindowHeight,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

    EnableWindow(MODE_CHANGE_BUTTON_SEND_MODE, (appMode != APPLICATION_MODE_SEND_MODE));
    EnableWindow(MODE_CHANGE_BUTTON_RECEIVE_MODE, (appMode != APPLICATION_MODE_RECEIVE_MODE));

    int receiveModeComponentShowMode = (appMode == APPLICATION_MODE_RECEIVE_MODE) ? SW_SHOW : SW_HIDE;
    ShowWindow(START_RECEIVING_BUTTON, receiveModeComponentShowMode);

    int sendModeComponentShowMode = (appMode == APPLICATION_MODE_SEND_MODE) ? SW_SHOW : SW_HIDE;
    ShowWindow(SEND_FILE_PATH_TEXTBOX, sendModeComponentShowMode);
    ShowWindow(SEND_FILE_PATH_BROWSE_BUTTON, sendModeComponentShowMode);
    ShowWindow(SEND_FILE_BUTTON, sendModeComponentShowMode);

    InvalidateRect(MAIN_WINDOW, NULL, FALSE);
}

BOOL GetSelectedPortName(wchar_t **resultPtr)
{
    int portListIndex = (int)SendMessageW(PORT_SELECT_COMBO_BOX, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    LRESULT portListData = SendMessageW(PORT_SELECT_COMBO_BOX, CB_GETITEMDATA, (WPARAM)portListIndex, (LPARAM)0);
    if (portListData == CB_ERR || portListData == 0)
    {
        return FALSE;
    }

    *resultPtr = (wchar_t *)portListData;

    return TRUE;
}

BOOL BrowseFileToSend(wchar_t *resultPtr)
{
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = MAIN_WINDOW;
    ofn.lpstrFile = resultPtr;
    ofn.nMaxFile = MAX_PATH_LENGTH;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    return GetOpenFileNameW(&ofn);
}

void UpdateSendFilePathTextbox(wchar_t *filePath)
{
    SetWindowTextW(SEND_FILE_PATH_TEXTBOX, filePath);
}

void GetSendFilePath(wchar_t *resultPtr)
{
    GetWindowTextW(SEND_FILE_PATH_TEXTBOX, resultPtr, MAX_PATH_LENGTH);
}

void EnableSetModeControls(BOOL enable)
{
    EnableWindow(PORT_SELECT_COMBO_BOX, enable);
    EnableWindow(PORT_SELECT_UPDATE_BUTTON, enable);
    EnableWindow(MODE_CHANGE_BUTTON_RECEIVE_MODE, enable);
    EnableWindow(SEND_FILE_PATH_TEXTBOX, enable);
    EnableWindow(SEND_FILE_PATH_BROWSE_BUTTON, enable);
    EnableWindow(SEND_FILE_BUTTON, enable);
}

void RequestErrorDialog(wchar_t *msg)
{
    wchar_t *persistentMsg = _wcsdup(msg);
    SendMessageW(MAIN_WINDOW, WM_SFTT_SHOW_ERROR_DIALOG, (WPARAM)persistentMsg, (LPARAM)0);
}

void UIStopReceiving(void)
{
    SetWindowTextW(START_RECEIVING_BUTTON, START_RECEIVING_BUTTON_LABEL_START);
}

void UIStartReceiving(void)
{
    SetWindowTextW(START_RECEIVING_BUTTON, START_RECEIVING_BUTTON_LABEL_STOP);
}

LRESULT CALLBACK MainWindowWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_SFTT_SHOW_ERROR_DIALOG: {
            wchar_t *msg = (wchar_t *)wParam;

            MessageBoxW(MAIN_WINDOW, msg, L"SFTT", MB_ICONERROR | MB_OK);

            free(msg);

            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);

            return 0;
        }
        case WM_PAINT: {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hwnd, &ps);

            HFONT oldFont = (HFONT)SelectObject(hdc, UI_FONT);

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

            HBRUSH backgroundBrush = GetSysColorBrush(COLOR_WINDOW);
            FillRect(hdc, &rect, backgroundBrush);

            return 1;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam))
            {
                case PORT_SELECT_UPDATE_BUTTON_ID: {
                    MAIN_LOGIC_SET.onPortSelectUpdateButton();

                    return 0;
                }
                case MODE_CHANGE_BUTTON_SEND_MODE_BUTTON_ID: {
                    MAIN_LOGIC_SET.onModeChangeButtonSendMode();

                    return 0;
                }
                case MODE_CHANGE_BUTTON_RECEIVE_MODE_BUTTON_ID: {
                    MAIN_LOGIC_SET.onModeChangeButtonReceiveMode();

                    return 0;
                }
                case START_RECEIVING_BUTTON_ID: {
                    MAIN_LOGIC_SET.onStartReceivingButton();

                    return 0;
                }
                case SEND_FILE_PATH_BROWSE_BUTTON_ID: {
                    MAIN_LOGIC_SET.onSendFilePathBrowseButton();

                    return 0;
                }
                case SEND_FILE_BUTTON_ID: {
                    MAIN_LOGIC_SET.onSendFileButton();

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

void ShowMainWindow(void)
{
    WNDCLASSEXW mainWindowClass;
    ZeroMemory(&mainWindowClass, sizeof(mainWindowClass));
    mainWindowClass.cbSize = sizeof(mainWindowClass);
    mainWindowClass.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    mainWindowClass.hInstance = MAIN_INSTANCE;
    mainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    mainWindowClass.lpfnWndProc = MainWindowWndProc;

    RegisterClassExW(&mainWindowClass);

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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
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
        MAIN_INSTANCE,
        NULL);

    SendMessageW(SEND_FILE_BUTTON, WM_SETFONT, (WPARAM)UI_FONT, (LPARAM)1);

    UpdatePortSelectList();

    SetApplicationMode(APPLICATION_MODE_SEND_MODE);

    ShowWindow(MAIN_WINDOW, SW_SHOWNORMAL);
    UpdateWindow(MAIN_WINDOW);
}

void FinaliseUI(void)
{
    FreePortSelectListPortNamePointers();

    if (UI_FONT != NULL)
    {
        DeleteObject(UI_FONT);
    }
}
