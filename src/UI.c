#define _WIN32_WINNT 0x0500

#include <shlobj.h>
#include <stdint.h>
#include <windows.h>

#include <commctrl.h>

#include "Error.h"
#include "Serial.h"
#include "UI.h"
#include "Util.h"

#define WM_SFTT_SHOW_ERROR_DIALOG WM_USER + 1
#define WM_SFTT_UI_START_SENDING WM_USER + 2
#define WM_SFTT_UI_FINISH_SENDING WM_USER + 3

#define UI_FONT_NAME L"Tahoma"
#define UI_FONT_SIZE 15

#define MAIN_WINDOW_CLASS_NAME L"SFTT_MAINWINDOW_CLASS"
#define MAIN_WINDOW_TITLE_SEND_MODE L"SFTT - Send"
#define MAIN_WINDOW_TITLE_RECEIVE_MODE L"SFTT - Receive"
#define MAIN_WINDOW_WIDTH 274
#define MAIN_WINDOW_HEIGHT_SEND_MODE 197
#define MAIN_WINDOW_HEIGHT_RECEIVE_MODE 197

#define PORT_SELECT_LABEL_TEXT L"Port: "
#define PORT_SELECT_LABEL_TEXT_LENGTH 6
#define PORT_SELECT_LABEL_X 5
#define PORT_SELECT_LABEL_Y 7

#define PORT_SELECT_COMBOBOX_X 35
#define PORT_SELECT_COMBOBOX_Y 4
#define PORT_SELECT_COMBOBOX_WIDTH 150
#define PORT_SELECT_COMBOBOX_HEIGHT 200

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

#define RECEIVE_DIRECTORY_LABEL_TEXT L"Dir: "
#define RECEIVE_DIRECTORY_LABEL_TEXT_LENGTH 5
#define RECEIVE_DIRECTORY_LABEL_X 10
#define RECEIVE_DIRECTORY_LABEL_Y 79

#define RECEIVE_DIRECTORY_TEXTBOX_X 35
#define RECEIVE_DIRECTORY_TEXTBOX_Y 76
#define RECEIVE_DIRECTORY_TEXTBOX_WIDTH 148
#define RECEIVE_DIRECTORY_TEXTBOX_HEIGHT 20

#define RECEIVE_DIRECTORY_BROWSE_BUTTON_LABEL L"Browse"
#define RECEIVE_DIRECTORY_BROWSE_BUTTON_X 190
#define RECEIVE_DIRECTORY_BROWSE_BUTTON_Y 76
#define RECEIVE_DIRECTORY_BROWSE_BUTTON_WIDTH 70
#define RECEIVE_DIRECTORY_BROWSE_BUTTON_HEIGHT 20
#define RECEIVE_DIRECTORY_BROWSE_BUTTON_ID 103
#define RECEIVE_DIRECTORY_BROWSE_DIALOG_TITLE L"Please select the directory for which received data will be stored"

#define START_RECEIVING_BUTTON_LABEL_START L"Start Receiving"
#define START_RECEIVING_BUTTON_LABEL_STOP L"Stop Receiving"
#define START_RECEIVING_BUTTON_X 7
#define START_RECEIVING_BUTTON_Y 103
#define START_RECEIVING_BUTTON_WIDTH 253
#define START_RECEIVING_BUTTON_HEIGHT 40
#define START_RECEIVING_BUTTON_ID 104

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
#define SEND_FILE_PATH_BROWSE_BUTTON_ID 105

#define SEND_FILE_BUTTON_LABEL L"Send a File"
#define SEND_FILE_BUTTON_X 7
#define SEND_FILE_BUTTON_Y 103
#define SEND_FILE_BUTTON_WIDTH 253
#define SEND_FILE_BUTTON_HEIGHT 40
#define SEND_FILE_BUTTON_ID 106

#define STATUS_BAR_TEXT_READY L"Ready"
#define STATUS_BAR_TEXT_SENDING L"Sending"
#define STATUS_BAR_TEXT_RECEIVING L"Receiving"

HINSTANCE g_mainInstance;

HFONT g_uiFont;

HWND g_mainWindow;
HWND g_mainWindowStatusBar;
HWND g_mainWindowStatusBarProgressBar;
HWND g_portSelectComboBox;
HWND g_portSelectUpdateButton;
HWND g_modeChangeButtonSendMode;
HWND g_modeChangeButtonReceiveMode;
HWND g_startReceivingButton;
HWND g_receiveDirectoryTextBox;
HWND g_receiveDirectoryBrowseButton;
HWND g_sendFilePathTextBox;
HWND g_sendFilePathBrowseButton;
HWND g_sendFileButton;

ApplicationMode g_currentApplicationMode;

void InitialiseUI(void)
{
    InitCommonControls();

    g_mainInstance = GetModuleHandleW(NULL);

    g_uiFont = CreateFontW(
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
    int itemCount = (int)SendMessageW(g_portSelectComboBox, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

    for (int i = 0; i < itemCount; i++)
    {
        LRESULT itemData = SendMessageW(g_portSelectComboBox, CB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
        if (itemData != CB_ERR && itemData != 0)
        {
            free((wchar_t *)itemData);
        }
    }

    SendMessageW(g_portSelectComboBox, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
}

void UpdatePortSelectList(void)
{
    EnableWindow(g_portSelectComboBox, FALSE);
    EnableWindow(g_portSelectUpdateButton, FALSE);

    FreePortSelectListPortNamePointers();

    AvailablePort availablePorts[COM_PORT_TRY_MAX];
    int numberOfAvailablePorts;
    GetAvailablePorts(availablePorts, &numberOfAvailablePorts);

    for (int i = 0; i < numberOfAvailablePorts; i++)
    {
        AvailablePort port = availablePorts[i];

        int itemIndex = (int)SendMessageW(g_portSelectComboBox, CB_ADDSTRING, (WPARAM)0, (LPARAM)port.friendlyPortName);
        SendMessageW(g_portSelectComboBox, CB_SETITEMDATA, (WPARAM)itemIndex, (LPARAM)port.portName);

        free(port.friendlyPortName);
    }

    EnableWindow(g_portSelectComboBox, TRUE);
    EnableWindow(g_portSelectUpdateButton, TRUE);
}

void SetApplicationMode(ApplicationMode appMode)
{
    g_currentApplicationMode = appMode;

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

    SendMessageW(g_mainWindow, WM_SETTEXT, (WPARAM)0, (LPARAM)newMainWindowTitle);
    SetWindowPos(
        g_mainWindow,
        NULL,
        0,
        0,
        MAIN_WINDOW_WIDTH,
        newMainWindowHeight,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

    EnableWindow(g_modeChangeButtonSendMode, (appMode != APPLICATION_MODE_SEND_MODE));
    EnableWindow(g_modeChangeButtonReceiveMode, (appMode != APPLICATION_MODE_RECEIVE_MODE));

    int receiveModeComponentShowMode = (appMode == APPLICATION_MODE_RECEIVE_MODE) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_startReceivingButton, receiveModeComponentShowMode);
    ShowWindow(g_receiveDirectoryTextBox, receiveModeComponentShowMode);
    ShowWindow(g_receiveDirectoryBrowseButton, receiveModeComponentShowMode);

    int sendModeComponentShowMode = (appMode == APPLICATION_MODE_SEND_MODE) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_sendFilePathTextBox, sendModeComponentShowMode);
    ShowWindow(g_sendFilePathBrowseButton, sendModeComponentShowMode);
    ShowWindow(g_sendFileButton, sendModeComponentShowMode);

    InvalidateRect(g_mainWindow, NULL, TRUE);
}

BOOL GetSelectedPortName(wchar_t **resultPtr)
{
    int portListIndex = (int)SendMessageW(g_portSelectComboBox, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    LRESULT portListData = SendMessageW(g_portSelectComboBox, CB_GETITEMDATA, (WPARAM)portListIndex, (LPARAM)0);
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
    ofn.hwndOwner = g_mainWindow;
    ofn.lpstrFile = resultPtr;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    return GetOpenFileNameW(&ofn);
}

BOOL BrowseReceiveDirectory(wchar_t *resultPtr)
{
    BROWSEINFOW bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = g_mainWindow;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi.lpszTitle = RECEIVE_DIRECTORY_BROWSE_DIALOG_TITLE;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);

    if (pidl != NULL)
    {
        if (SHGetPathFromIDListW(pidl, resultPtr))
        {
            CoTaskMemFree((LPVOID)pidl);

            return TRUE;
        }

        CoTaskMemFree((LPVOID)pidl);
    }

    return FALSE;
}

void UpdateSendFilePathTextbox(wchar_t *filePath)
{
    SetWindowTextW(g_sendFilePathTextBox, filePath);
}

void GetSendFilePath(wchar_t *resultPtr)
{
    GetWindowTextW(g_sendFilePathTextBox, resultPtr, MAX_PATH);
}

void UpdateReceiveDirectoryTextbox(wchar_t *dirPath)
{
    SetWindowTextW(g_receiveDirectoryTextBox, dirPath);
}

void GetReceiveDirectory(wchar_t *resultPtr)
{
    GetWindowTextW(g_receiveDirectoryTextBox, resultPtr, MAX_PATH);
}

void EnableSetModeControls(BOOL enable)
{
    EnableWindow(g_portSelectComboBox, enable);
    EnableWindow(g_portSelectUpdateButton, enable);
    EnableWindow(g_modeChangeButtonReceiveMode, enable);
    EnableWindow(g_sendFilePathTextBox, enable);
    EnableWindow(g_sendFilePathBrowseButton, enable);
    EnableWindow(g_sendFileButton, enable);
}

void RequestErrorDialog(wchar_t *msg)
{
    wchar_t *persistentMsg = _wcsdup(msg);
    SendMessageW(g_mainWindow, WM_SFTT_SHOW_ERROR_DIALOG, (WPARAM)persistentMsg, (LPARAM)0);
}

void UIStopReceiving(void)
{
    SetWindowTextW(g_startReceivingButton, START_RECEIVING_BUTTON_LABEL_START);

    EnableWindow(g_portSelectComboBox, TRUE);
    EnableWindow(g_portSelectUpdateButton, TRUE);
    EnableWindow(g_modeChangeButtonSendMode, TRUE);
    EnableWindow(g_receiveDirectoryBrowseButton, TRUE);
    EnableWindow(g_receiveDirectoryTextBox, TRUE);
}

void UIStartReceiving(void)
{
    SetWindowTextW(g_startReceivingButton, START_RECEIVING_BUTTON_LABEL_STOP);

    EnableWindow(g_portSelectComboBox, FALSE);
    EnableWindow(g_portSelectUpdateButton, FALSE);
    EnableWindow(g_modeChangeButtonSendMode, FALSE);
    EnableWindow(g_receiveDirectoryBrowseButton, FALSE);
    EnableWindow(g_receiveDirectoryTextBox, FALSE);
}

void SetStatusBarText(StatusBarStatus status)
{
    wchar_t *newStatusBarText = L"";
    switch (status)
    {
        case STATUS_BAR_STATUS_READY: {
            newStatusBarText = STATUS_BAR_TEXT_READY;
            break;
        }
        case STATUS_BAR_STATUS_RECEIVING: {
            newStatusBarText = STATUS_BAR_TEXT_RECEIVING;
            break;
        }
        case STATUS_BAR_STATUS_SENDING: {
            newStatusBarText = STATUS_BAR_TEXT_SENDING;
            break;
        }
    }

    SendMessageW(g_mainWindowStatusBar, SB_SETTEXTW, 0 | 0, (LPARAM)newStatusBarText);
}

void SetProgressBarRange(uint32_t max)
{
    SendMessageW(g_mainWindowStatusBarProgressBar, PBM_SETRANGE32, (WPARAM)0, (LPARAM)max);
    SendMessageW(g_mainWindowStatusBarProgressBar, PBM_SETSTEP, (WPARAM)1, (LPARAM)0);
}

void StepProgressBar(void)
{
    SendMessageW(g_mainWindowStatusBarProgressBar, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
}

void AddStepToProgressBar(uint32_t steps)
{
    uint32_t current = (uint32_t)SendMessageW(g_mainWindowStatusBarProgressBar, PBM_GETPOS, (WPARAM)0, (LPARAM)0);
    SendMessageW(g_mainWindowStatusBarProgressBar, PBM_SETPOS, (WPARAM)current + steps, (LPARAM)0);
}

void ResetProgressBar(void)
{
    SendMessageW(g_mainWindowStatusBarProgressBar, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
}

void UIStartSending(void)
{
    SendMessageW(g_mainWindow, WM_SFTT_UI_START_SENDING, (WPARAM)0, (LPARAM)0);
}

void UIFinishSending(void)
{
    SendMessageW(g_mainWindow, WM_SFTT_UI_FINISH_SENDING, (WPARAM)0, (LPARAM)0);
}

LRESULT CALLBACK MainWindowWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_SFTT_SHOW_ERROR_DIALOG: {
            wchar_t *msg = (wchar_t *)wParam;

            MessageBoxW(g_mainWindow, msg, L"SFTT", MB_ICONERROR | MB_OK);

            free(msg);

            return 0;
        }
        case WM_SFTT_UI_START_SENDING: {
            EnableSetModeControls(FALSE);
            SetStatusBarText(STATUS_BAR_STATUS_SENDING);

            return 0;
        }
        case WM_SFTT_UI_FINISH_SENDING: {
            EnableSetModeControls(TRUE);
            SetStatusBarText(STATUS_BAR_STATUS_READY);

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

            HFONT oldFont = (HFONT)SelectObject(hdc, g_uiFont);

            TextOutW(
                hdc,
                PORT_SELECT_LABEL_X,
                PORT_SELECT_LABEL_Y,
                PORT_SELECT_LABEL_TEXT,
                PORT_SELECT_LABEL_TEXT_LENGTH);

            switch (g_currentApplicationMode)
            {
                case APPLICATION_MODE_SEND_MODE: {
                    TextOutW(
                        hdc,
                        FILE_TO_SEND_LABEL_X,
                        FILE_TO_SEND_LABEL_Y,
                        FILE_TO_SEND_LABEL_TEXT,
                        FILE_TO_SEND_LABEL_TEXT_LENGTH);

                    break;
                }
                case APPLICATION_MODE_RECEIVE_MODE: {
                    TextOutW(
                        hdc,
                        RECEIVE_DIRECTORY_LABEL_X,
                        RECEIVE_DIRECTORY_LABEL_Y,
                        RECEIVE_DIRECTORY_LABEL_TEXT,
                        RECEIVE_DIRECTORY_LABEL_TEXT_LENGTH);

                    break;
                }
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
        case WM_SIZE: {
            SendMessageW(g_mainWindowStatusBar, wMsg, wParam, lParam);

            return 0;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam))
            {
                case PORT_SELECT_UPDATE_BUTTON_ID: {
                    UpdatePortSelectList();

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
                    if (IsReceiving())
                    {
                        StopReceiving();
                        UIStopReceiving();
                    }
                    else
                    {
                        wchar_t *selectedPortName;
                        if (!GetSelectedPortName(&selectedPortName))
                        {
                            PleaseSpecifyPortError();

                            return 0;
                        }

                        if (StartReceiving(selectedPortName))
                        {
                            UIStartReceiving();
                        }
                        else
                        {
                            StopReceiving();
                            UIStopReceiving();

                            CannotOpenCOMPortError(selectedPortName);

                            return 0;
                        }
                    }

                    return 0;
                }
                case SEND_FILE_PATH_BROWSE_BUTTON_ID: {
                    wchar_t filePath[MAX_PATH] = L"";

                    if (BrowseFileToSend(filePath))
                    {
                        UpdateSendFilePathTextbox(filePath);
                    }

                    return 0;
                }
                case RECEIVE_DIRECTORY_BROWSE_BUTTON_ID: {
                    wchar_t dirPath[MAX_PATH] = L"";

                    if (BrowseReceiveDirectory(dirPath))
                    {
                        UpdateReceiveDirectoryTextbox(dirPath);
                    }

                    return 0;
                }
                case SEND_FILE_BUTTON_ID: {
                    wchar_t *selectedPortName;
                    if (!GetSelectedPortName(&selectedPortName))
                    {
                        PleaseSpecifyPortError();

                        return 0;
                    }

                    wchar_t filePath[MAX_PATH];
                    GetSendFilePath(filePath);
                    if (wcslen(filePath) == 0)
                    {
                        PleaseSpecifyFilePathError();

                        return 0;
                    }

                    SendFile(selectedPortName, filePath);

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
    mainWindowClass.hInstance = g_mainInstance;
    mainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    mainWindowClass.lpfnWndProc = MainWindowWndProc;

    RegisterClassExW(&mainWindowClass);

    g_mainWindow = CreateWindowW(
        MAIN_WINDOW_CLASS_NAME,
        NULL,
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        MAIN_WINDOW_WIDTH,
        0,
        NULL,
        NULL,
        g_mainInstance,
        NULL);

    g_portSelectComboBox = CreateWindowW(
        L"COMBOBOX",
        NULL,
        CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL | WS_VISIBLE,
        PORT_SELECT_COMBOBOX_X,
        PORT_SELECT_COMBOBOX_Y,
        PORT_SELECT_COMBOBOX_WIDTH,
        PORT_SELECT_COMBOBOX_HEIGHT,
        g_mainWindow,
        NULL,
        g_mainInstance,
        NULL);

    SendMessageW(g_portSelectComboBox, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_portSelectUpdateButton = CreateWindowW(
        L"BUTTON",
        PORT_SELECT_UPDATE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        PORT_SELECT_UPDATE_BUTTON_X,
        PORT_SELECT_UPDATE_BUTTON_Y,
        PORT_SELECT_UPDATE_BUTTON_WIDTH,
        PORT_SELECT_UPDATE_BUTTON_HEIGHT,
        g_mainWindow,
        (HMENU)PORT_SELECT_UPDATE_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_portSelectUpdateButton, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_modeChangeButtonSendMode = CreateWindowW(
        L"BUTTON",
        MODE_CHANGE_BUTTON_SEND_MODE_LABEL,
        WS_CHILD | WS_VISIBLE,
        MODE_CHANGE_BUTTON_SEND_MODE_X,
        MODE_CHANGE_BUTTON_SEND_MODE_Y,
        MODE_CHANGE_BUTTON_SEND_MODE_WIDTH,
        MODE_CHANGE_BUTTON_SEND_MODE_HEIGHT,
        g_mainWindow,
        (HMENU)MODE_CHANGE_BUTTON_SEND_MODE_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_modeChangeButtonSendMode, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_modeChangeButtonReceiveMode = CreateWindowW(
        L"BUTTON",
        MODE_CHANGE_BUTTON_RECEIVE_MODE_LABEL,
        WS_CHILD | WS_VISIBLE,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_X,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_Y,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_WIDTH,
        MODE_CHANGE_BUTTON_RECEIVE_MODE_HEIGHT,
        g_mainWindow,
        (HMENU)MODE_CHANGE_BUTTON_RECEIVE_MODE_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_modeChangeButtonReceiveMode, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_startReceivingButton = CreateWindowW(
        L"BUTTON",
        START_RECEIVING_BUTTON_LABEL_START,
        WS_CHILD | WS_VISIBLE,
        START_RECEIVING_BUTTON_X,
        START_RECEIVING_BUTTON_Y,
        START_RECEIVING_BUTTON_WIDTH,
        START_RECEIVING_BUTTON_HEIGHT,
        g_mainWindow,
        (HMENU)START_RECEIVING_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_startReceivingButton, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_receiveDirectoryTextBox = CreateWindowW(
        L"EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        RECEIVE_DIRECTORY_TEXTBOX_X,
        RECEIVE_DIRECTORY_TEXTBOX_Y,
        RECEIVE_DIRECTORY_TEXTBOX_WIDTH,
        RECEIVE_DIRECTORY_TEXTBOX_HEIGHT,
        g_mainWindow,
        NULL,
        g_mainInstance,
        NULL);

    SendMessageW(g_receiveDirectoryTextBox, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    SetWindowTextW(g_receiveDirectoryTextBox, currentDir);

    g_receiveDirectoryBrowseButton = CreateWindowW(
        L"BUTTON",
        RECEIVE_DIRECTORY_BROWSE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        RECEIVE_DIRECTORY_BROWSE_BUTTON_X,
        RECEIVE_DIRECTORY_BROWSE_BUTTON_Y,
        RECEIVE_DIRECTORY_BROWSE_BUTTON_WIDTH,
        RECEIVE_DIRECTORY_BROWSE_BUTTON_HEIGHT,
        g_mainWindow,
        (HMENU)RECEIVE_DIRECTORY_BROWSE_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_receiveDirectoryBrowseButton, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_sendFilePathTextBox = CreateWindowW(
        L"EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        SEND_FILE_PATH_TEXTBOX_X,
        SEND_FILE_PATH_TEXTBOX_Y,
        SEND_FILE_PATH_TEXTBOX_WIDTH,
        SEND_FILE_PATH_TEXTBOX_HEIGHT,
        g_mainWindow,
        NULL,
        g_mainInstance,
        NULL);

    SendMessageW(g_sendFilePathTextBox, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_sendFilePathBrowseButton = CreateWindowW(
        L"BUTTON",
        SEND_FILE_PATH_BROWSE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        SEND_FILE_PATH_BROWSE_BUTTON_X,
        SEND_FILE_PATH_BROWSE_BUTTON_Y,
        SEND_FILE_PATH_BROWSE_BUTTON_WIDTH,
        SEND_FILE_PATH_BROWSE_BUTTON_HEIGHT,
        g_mainWindow,
        (HMENU)SEND_FILE_PATH_BROWSE_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_sendFilePathBrowseButton, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_sendFileButton = CreateWindowW(
        L"BUTTON",
        SEND_FILE_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        SEND_FILE_BUTTON_X,
        SEND_FILE_BUTTON_Y,
        SEND_FILE_BUTTON_WIDTH,
        SEND_FILE_BUTTON_HEIGHT,
        g_mainWindow,
        (HMENU)SEND_FILE_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(g_sendFileButton, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    g_mainWindowStatusBar = CreateWindowW(
        STATUSCLASSNAMEW,
        NULL,
        CCS_BOTTOM | WS_CHILD | WS_VISIBLE,
        0,
        0,
        0,
        0,
        g_mainWindow,
        NULL,
        g_mainInstance,
        NULL);

    int parts[2] = {MAIN_WINDOW_WIDTH / 3, -1};
    SendMessageW(g_mainWindowStatusBar, SB_SETPARTS, (WPARAM)2, (LPARAM)parts);

    SendMessageW(g_mainWindowStatusBar, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    RECT rcPart;
    SendMessageW(g_mainWindowStatusBar, SB_GETRECT, (WPARAM)1, (LPARAM)&rcPart);

    g_mainWindowStatusBarProgressBar = CreateWindowW(
        PROGRESS_CLASSW,
        NULL,
        WS_CHILD | WS_VISIBLE,
        rcPart.left + 2,
        rcPart.top + 2,
        (rcPart.right - rcPart.left) - 4,
        (rcPart.bottom - rcPart.top) - 4,
        g_mainWindowStatusBar,
        NULL,
        g_mainInstance,
        NULL);

    SetStatusBarText(STATUS_BAR_STATUS_READY);

    UpdatePortSelectList();

    SetApplicationMode(APPLICATION_MODE_SEND_MODE);

    ShowWindow(g_mainWindow, SW_SHOWNORMAL);
    UpdateWindow(g_mainWindow);
}

void FinaliseUI(void)
{
    FreePortSelectListPortNamePointers();

    if (g_uiFont != NULL)
    {
        DeleteObject(g_uiFont);
    }
}
