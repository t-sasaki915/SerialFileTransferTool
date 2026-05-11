#define _WIN32_WINNT 0x0500

#include <shlobj.h>
#include <stdint.h>
#include <windows.h>

#include <commctrl.h>

#include "Error.h"
#include "Serial.h"
#include "UI.h"
#include "Util.h"
#include "Version.h"

#define WM_SFTT_SHOW_ERROR_DIALOG WM_USER + 1
#define WM_SFTT_UI_START_SENDING WM_USER + 2
#define WM_SFTT_UI_FINISH_SENDING WM_USER + 3
#define WM_SFTT_ENABLE_START_RECEIVING_BUTTON WM_USER + 4

#define UI_FONT_NAME L"Tahoma"
#define UI_FONT_SIZE 15

#define VERSION_WINDOW_TITLE L"About SFTT"

#define MAIN_WINDOW_CLASS_NAME L"SFTT_MAINWINDOW_CLASS"
#define MAIN_WINDOW_TITLE_SEND_MODE L"SFTT - Send"
#define MAIN_WINDOW_TITLE_RECEIVE_MODE L"SFTT - Receive"
#define MAIN_WINDOW_WIDTH 274
#define MAIN_WINDOW_HEIGHT 217

#define PORT_SELECT_LABEL_TEXT L"Port:"
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

#define RECEIVE_DIRECTORY_LABEL_TEXT L"Dest:"
#define RECEIVE_DIRECTORY_LABEL_TEXT_LENGTH 5
#define RECEIVE_DIRECTORY_LABEL_X 3
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
#define RECEIVE_DIRECTORY_BROWSE_DIALOG_TITLE L"Please select the directory in which received data will be stored"

#define START_RECEIVING_BUTTON_LABEL_START L"Start Receiving"
#define START_RECEIVING_BUTTON_LABEL_STOP L"Stop Receiving"
#define START_RECEIVING_BUTTON_X 7
#define START_RECEIVING_BUTTON_Y 103
#define START_RECEIVING_BUTTON_WIDTH 253
#define START_RECEIVING_BUTTON_HEIGHT 40
#define START_RECEIVING_BUTTON_ID 104

#define FILE_TO_SEND_LABEL_TEXT L"File:"
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

#define FILE_MENU_LABEL L"File (&F)"

#define FILE_MENU_VERSION_LABEL L"Version (&V)"
#define FILE_MENU_VERSION_ID 107

#define FILE_MENU_EXIT_LABEL L"Exit (&X)"
#define FILE_MENU_EXIT_ID 108

#define BAUD_RATE_MENU_LABEL L"Baud Rate (&B)"
#define BAUD_RATE_MENU_ID 109

#define BAUD_RATE_SETTING_WINDOW_CLASS_NAME L"SFTT_BAUD_RATE_SETTING_WINDOW_CLASS"
#define BAUD_RATE_SETTING_WINDOW_TITLE L"Baud Rate Setting"
#define BAUD_RATE_SETTING_WINDOW_WIDTH 232
#define BAUD_RATE_SETTING_WINDOW_HEIGHT 89

#define BAUD_RATE_LABEL_TEXT L"Baud Rate:"
#define BAUD_RATE_LABEL_TEXT_LENGTH 11
#define BAUD_RATE_LABEL_X 5
#define BAUD_RATE_LABEL_Y 7

#define BAUD_RATE_TEXTBOX_X 72
#define BAUD_RATE_TEXTBOX_Y 5
#define BAUD_RATE_TEXTBOX_WIDTH 148
#define BAUD_RATE_TEXTBOX_HEIGHT 20
#define BAUD_RATE_TEXTBOX_ID 110

#define BAUD_RATE_OK_BUTTON_LABEL L"OK"
#define BAUD_RATE_OK_BUTTON_X 77
#define BAUD_RATE_OK_BUTTON_Y 35
#define BAUD_RATE_OK_BUTTON_WIDTH 70
#define BAUD_RATE_OK_BUTTON_HEIGHT 20
#define BAUD_RATE_OK_BUTTON_ID 111

#define PLEASE_ENTER_BAUD_RATE_MSG L"Please enter a baud rate."
#define BAUD_RATE_OUT_OF_RANGE_MSG L"The baud rate is out of range."

static HINSTANCE g_mainInstance;

static HFONT g_uiFont;

static HWND g_mainWindow;
static HWND g_mainWindowStatusBar;
static HWND g_mainWindowStatusBarProgressBar;
static HWND g_portSelectComboBox;
static HWND g_portSelectUpdateButton;
static HWND g_modeChangeButtonSendMode;
static HWND g_modeChangeButtonReceiveMode;
static HWND g_startReceivingButton;
static HWND g_receiveDirectoryTextBox;
static HWND g_receiveDirectoryBrowseButton;
static HWND g_sendFilePathTextBox;
static HWND g_sendFilePathBrowseButton;
static HWND g_sendFileButton;

static HMENU g_mainWindowMenu;

static HWND g_baudRateSettingWindow;

static ApplicationMode g_currentApplicationMode;

void InitialiseUI(void)
{
    InitCommonControls();
    OleInitialize(NULL);

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

    wchar_t *newMainWindowTitle = L"";
    switch (appMode)
    {
        case APPLICATION_MODE_SEND_MODE: {
            newMainWindowTitle = MAIN_WINDOW_TITLE_SEND_MODE;

            break;
        }
        case APPLICATION_MODE_RECEIVE_MODE: {
            newMainWindowTitle = MAIN_WINDOW_TITLE_RECEIVE_MODE;

            break;
        }
    }

    SetWindowTextW(g_mainWindow, newMainWindowTitle);

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
    OPENFILENAMEW ofn = {0};
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
    BROWSEINFOW bi = {0};
    bi.hwndOwner = g_mainWindow;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi.lpszTitle = RECEIVE_DIRECTORY_BROWSE_DIALOG_TITLE;

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

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
    SendMessageW(g_mainWindow, WM_SFTT_SHOW_ERROR_DIALOG, (WPARAM)_wcsdup(msg), (LPARAM)0);
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

void AddStepsToProgressBar(uint32_t steps)
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

void EnableStartReceivingButton(BOOL enable)
{
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_START_RECEIVING_BUTTON, (WPARAM)enable, (LPARAM)0);
}

void ShowVersion(void)
{
    wchar_t *versionText =
        L"SerialFileTransferTool v" SFTT_VERSION "\r\n\r\n" SFTT_LICENCE "\r\n\r\nGitHub: " SFTT_GITHUB;

    MessageBoxW(g_mainWindow, versionText, VERSION_WINDOW_TITLE, MB_ICONINFORMATION | MB_OK);
}

void EnableBaudRateSettingButton(BOOL enable)
{
    EnableMenuItem(g_mainWindowMenu, BAUD_RATE_MENU_ID, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED));
    DrawMenuBar(g_mainWindow);
}

LRESULT CALLBACK BaudRateSettingWindowWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_DESTROY: {
            EnableWindow(g_mainWindow, TRUE);
            SetForegroundWindow(g_mainWindow);

            return 0;
        }
        case WM_PAINT: {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hwnd, &ps);

            HFONT oldFont = (HFONT)SelectObject(hdc, g_uiFont);

            TextOutW(hdc, BAUD_RATE_LABEL_X, BAUD_RATE_LABEL_Y, BAUD_RATE_LABEL_TEXT, BAUD_RATE_LABEL_TEXT_LENGTH);

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
                case BAUD_RATE_OK_BUTTON_ID: {
                    BOOL result;
                    UINT newBaudRate = GetDlgItemInt(g_baudRateSettingWindow, BAUD_RATE_TEXTBOX_ID, &result, FALSE);

                    if (!result)
                    {
                        MessageBoxW(
                            g_baudRateSettingWindow,
                            PLEASE_ENTER_BAUD_RATE_MSG,
                            BAUD_RATE_SETTING_WINDOW_TITLE,
                            MB_ICONERROR | MB_OK);

                        return 0;
                    }

                    if (newBaudRate <= 0 || newBaudRate > BAUD_RATE_MAX)
                    {
                        MessageBoxW(
                            g_baudRateSettingWindow,
                            BAUD_RATE_OUT_OF_RANGE_MSG,
                            BAUD_RATE_SETTING_WINDOW_TITLE,
                            MB_ICONERROR | MB_OK);

                        return 0;
                    }

                    SetBaudRate((DWORD)newBaudRate);

                    PostMessageW(g_baudRateSettingWindow, WM_CLOSE, (WPARAM)0, (LPARAM)0);

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

void ShowBaudRateSettingWindow(void)
{
    WNDCLASSEXW wndClass = {0};
    wndClass.cbSize = sizeof(wndClass);
    wndClass.lpszClassName = BAUD_RATE_SETTING_WINDOW_CLASS_NAME;
    wndClass.hInstance = g_mainInstance;
    wndClass.style = CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc = BaudRateSettingWindowWndProc;

    RegisterClassExW(&wndClass);

    EnableWindow(g_mainWindow, FALSE);

    RECT mainWindowRect;
    GetWindowRect(g_mainWindow, &mainWindowRect);

    g_baudRateSettingWindow = CreateWindowW(
        BAUD_RATE_SETTING_WINDOW_CLASS_NAME,
        BAUD_RATE_SETTING_WINDOW_TITLE,
        WS_CAPTION | WS_SYSMENU | WS_POPUPWINDOW | WS_DLGFRAME,
        mainWindowRect.left,
        mainWindowRect.top,
        BAUD_RATE_SETTING_WINDOW_WIDTH,
        BAUD_RATE_SETTING_WINDOW_HEIGHT,
        NULL,
        NULL,
        g_mainInstance,
        NULL);

    wchar_t textBoxInitContent[7];
    Format(textBoxInitContent, 7, L"%lu", GetCurrentBaudRate());

    HWND baudRateTextBox = CreateWindowW(
        L"EDIT",
        textBoxInitContent,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
        BAUD_RATE_TEXTBOX_X,
        BAUD_RATE_TEXTBOX_Y,
        BAUD_RATE_TEXTBOX_WIDTH,
        BAUD_RATE_TEXTBOX_HEIGHT,
        g_baudRateSettingWindow,
        (HMENU)BAUD_RATE_TEXTBOX_ID,
        g_mainInstance,
        NULL);

    SendMessageW(baudRateTextBox, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    HWND baudRateOKButton = CreateWindowW(
        L"BUTTON",
        BAUD_RATE_OK_BUTTON_LABEL,
        WS_CHILD | WS_VISIBLE,
        BAUD_RATE_OK_BUTTON_X,
        BAUD_RATE_OK_BUTTON_Y,
        BAUD_RATE_OK_BUTTON_WIDTH,
        BAUD_RATE_OK_BUTTON_HEIGHT,
        g_baudRateSettingWindow,
        (HMENU)BAUD_RATE_OK_BUTTON_ID,
        g_mainInstance,
        NULL);

    SendMessageW(baudRateOKButton, WM_SETFONT, (WPARAM)g_uiFont, (LPARAM)1);

    ShowWindow(g_baudRateSettingWindow, SW_SHOWNORMAL);
    UpdateWindow(g_baudRateSettingWindow);
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
        case WM_SFTT_ENABLE_START_RECEIVING_BUTTON: {
            BOOL enable = (BOOL)wParam;

            EnableWindow(g_startReceivingButton, enable);

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
                case FILE_MENU_VERSION_ID: {
                    ShowVersion();

                    return 0;
                }
                case FILE_MENU_EXIT_ID: {
                    PostQuitMessage(0);

                    return 0;
                }
                case BAUD_RATE_MENU_ID: {
                    ShowBaudRateSettingWindow();

                    return 0;
                }
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
                        EnableBaudRateSettingButton(TRUE);
                    }
                    else
                    {
                        wchar_t *selectedPortName;
                        if (!GetSelectedPortName(&selectedPortName))
                        {
                            PleaseSpecifyPortError();

                            return 0;
                        }

                        wchar_t receiveDir[MAX_PATH];
                        GetReceiveDirectory(receiveDir);
                        if (wcslen(receiveDir) == 0)
                        {
                            PleaseSpecifyDirectoryError();

                            return 0;
                        }

                        if (StartReceiving(selectedPortName, receiveDir))
                        {
                            UIStartReceiving();
                            EnableBaudRateSettingButton(FALSE);
                        }
                        else
                        {
                            StopReceiving();
                            UIStopReceiving();
                            EnableBaudRateSettingButton(TRUE);

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
    WNDCLASSEXW mainWindowClass = {0};
    mainWindowClass.cbSize = sizeof(mainWindowClass);
    mainWindowClass.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    mainWindowClass.hInstance = g_mainInstance;
    mainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    mainWindowClass.lpfnWndProc = MainWindowWndProc;

    RegisterClassExW(&mainWindowClass);

    g_mainWindowMenu = CreateMenu();

    HMENU fileMenu = CreateMenu();
    AppendMenuW(fileMenu, MF_STRING, (UINT_PTR)FILE_MENU_VERSION_ID, FILE_MENU_VERSION_LABEL);
    AppendMenuW(fileMenu, MF_SEPARATOR, (UINT_PTR)0, NULL);
    AppendMenuW(fileMenu, MF_STRING, (UINT_PTR)FILE_MENU_EXIT_ID, FILE_MENU_EXIT_LABEL);

    AppendMenuW(g_mainWindowMenu, MF_POPUP, (UINT_PTR)fileMenu, FILE_MENU_LABEL);
    AppendMenuW(g_mainWindowMenu, MF_STRING, (UINT_PTR)BAUD_RATE_MENU_ID, BAUD_RATE_MENU_LABEL);

    g_mainWindow = CreateWindowW(
        MAIN_WINDOW_CLASS_NAME,
        NULL,
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        MAIN_WINDOW_WIDTH,
        MAIN_WINDOW_HEIGHT,
        NULL,
        g_mainWindowMenu,
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
