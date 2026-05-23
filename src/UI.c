#define _WIN32_WINNT 0x0500

#include <shlobj.h>
#include <stdint.h>
#include <windows.h>

#include <commctrl.h>

#include "Error.h"
#include "Resource.h"
#include "Serial.h"
#include "UI.h"
#include "Util.h"

#define WM_SFTT_SHOW_ERROR_DIALOG WM_USER + 1
#define WM_SFTT_UI_START_SENDING WM_USER + 2
#define WM_SFTT_UI_FINISH_SENDING WM_USER + 3
#define WM_SFTT_ENABLE_START_RECEIVING_BUTTON WM_USER + 4

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
#define FILE_TO_SEND_LABEL_TEXT_LENGTH 5
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
#define BAUD_RATE_LABEL_TEXT_LENGTH 10
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

static HWND g_mainWindow;
static HWND g_mainWindowStatusBar;
static HWND g_mainWindowStatusBarProgressBar;
static HWND g_portSelectComboBox;
static HWND g_portSelectUpdateButton;
static HWND g_modeChangeButtonSendMode;
static HWND g_modeChangeButtonReceiveMode;
static HWND g_targetPathTextBox;
static HWND g_targetBrowseButton;
static HWND g_executeButton;
static HWND g_startReceivingButton;
static HWND g_receiveDirectoryTextBox;
static HWND g_receiveDirectoryBrowseButton;
static HWND g_sendFilePathTextBox;
static HWND g_sendFilePathBrowseButton;
static HWND g_sendFileButton;

static wchar_t *g_lastSendModeTargetText = NULL;
static wchar_t *g_lastReceiveModeTargetText = NULL;

static HMENU g_mainWindowMenu;

static HWND g_baudRateSettingWindow;

static ApplicationMode g_currentApplicationMode;

void InitialiseUI(void)
{
    InitCommonControls();
    OleInitialize(NULL);

    g_mainInstance = GetModuleHandleW(NULL);

    g_lastSendModeTargetText = _wcsdup(L"");
    g_lastReceiveModeTargetText = _wcsdup(L"");
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

void SetApplicationMode(ApplicationMode newAppMode)
{
    g_currentApplicationMode = newAppMode;

    wchar_t newMainWindowTitle[256];
    wchar_t newTargetLabelText[256];
    wchar_t newExecuteButtonText[256];
    wchar_t *newTargetPath = NULL;
    switch (newAppMode)
    {
        case APPLICATION_MODE_SEND_MODE: {
            LoadStringW(g_mainInstance, IDSTRING_MAIN_DIALOG_TITLE_SEND_MODE, newMainWindowTitle, 256);
            LoadStringW(g_mainInstance, IDSTRING_TARGET_LABEL_SEND_MODE, newTargetLabelText, 256);
            LoadStringW(g_mainInstance, IDSTRING_EXECUTE_BUTTON_LABEL_SEND_MODE, newExecuteButtonText, 256);

            wchar_t currentPath[MAX_PATH];
            GetTargetPath(currentPath);

            if (g_lastReceiveModeTargetText != NULL)
            {
                free(g_lastReceiveModeTargetText);
                g_lastReceiveModeTargetText = NULL;
            }
            g_lastReceiveModeTargetText = _wcsdup(currentPath);

            newTargetPath = g_lastSendModeTargetText;

            break;
        }
        case APPLICATION_MODE_RECEIVE_MODE: {
            LoadStringW(g_mainInstance, IDSTRING_MAIN_DIALOG_TITLE_RECEIVE_MODE, newMainWindowTitle, 256);
            LoadStringW(g_mainInstance, IDSTRING_TARGET_LABEL_RECEIVE_MODE, newTargetLabelText, 256);
            LoadStringW(g_mainInstance, IDSTRING_EXECUTE_BUTTON_LABEL_RECEIVE_MODE, newExecuteButtonText, 256);

            wchar_t currentPath[MAX_PATH];
            GetTargetPath(currentPath);

            if (g_lastSendModeTargetText != NULL)
            {
                free(g_lastSendModeTargetText);
                g_lastSendModeTargetText = NULL;
            }
            g_lastSendModeTargetText = _wcsdup(currentPath);

            newTargetPath = g_lastReceiveModeTargetText;

            break;
        }
    }

    SetWindowTextW(g_mainWindow, newMainWindowTitle);
    SetDlgItemTextW(g_mainWindow, IDSTATIC_TARGET, newTargetLabelText);
    SetWindowTextW(g_executeButton, newExecuteButtonText);

    if (newTargetPath != NULL)
    {
        SetWindowTextW(g_targetPathTextBox, newTargetPath);
    }

    EnableWindow(g_modeChangeButtonSendMode, (newAppMode != APPLICATION_MODE_SEND_MODE));
    EnableWindow(g_modeChangeButtonReceiveMode, (newAppMode != APPLICATION_MODE_RECEIVE_MODE));
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

void GetTargetPath(wchar_t *resultPtr)
{
    GetWindowTextW(g_targetPathTextBox, resultPtr, MAX_PATH);
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

void RequestErrorDialog(ErrorContext *errorContext)
{
    SendMessageW(g_mainWindow, WM_SFTT_SHOW_ERROR_DIALOG, (WPARAM)errorContext, (LPARAM)0);
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
    wchar_t versionTextFormat[512];
    LoadStringW(g_mainInstance, IDSTRING_VERSION_DIALOG_TEXT, versionTextFormat, 512);
    wchar_t sfttVersion[20];
    LoadStringW(g_mainInstance, IDSTRING_SFTT_VERSION, sfttVersion, 20);
    wchar_t sfttLicence[128];
    LoadStringW(g_mainInstance, IDSTRING_SFTT_LICENCE, sfttLicence, 128);
    wchar_t sfttGitHub[256];
    LoadStringW(g_mainInstance, IDSTRING_SFTT_GITHUB, sfttGitHub, 256);

    wchar_t versionText[512];
    Format(versionText, 512, versionTextFormat, sfttVersion, sfttLicence, sfttGitHub);
    wchar_t dialogTitle[128];
    LoadStringW(g_mainInstance, IDSTRING_VERSION_DIALOG_TITLE, dialogTitle, 128);

    MessageBoxW(g_mainWindow, versionText, dialogTitle, MB_ICONINFORMATION | MB_OK);
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
    /*WNDCLASSEXW wndClass = {0};
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
    UpdateWindow(g_baudRateSettingWindow);*/
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
        case WM_COMMAND: {
            switch (LOWORD(wParam))
            {
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
                    GetTargetPath(filePath);
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

INT_PTR CALLBACK MainDialogDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;

    switch (wMsg)
    {
        case WM_SFTT_SHOW_ERROR_DIALOG: {
            ErrorContext *ctx = (ErrorContext *)wParam;

            wchar_t errorDialogTitle[32];
            LoadStringW(g_mainInstance, IDSTRING_ERROR_DIALOG_TITLE, errorDialogTitle, 32);

            wchar_t errorMsg[1024];
            wchar_t errorMsgFormat[1024];
            LoadStringW(g_mainInstance, ctx->errorType, errorMsgFormat, 1024);

            switch (ctx->errorType)
            {
                case ERROR_TYPE_CANNOT_OPEN_FILE:
                case ERROR_TYPE_CANNOT_GET_FILE_SIZE: {
                    Format(errorMsg, 1024, errorMsgFormat, ctx->errorDetails.filePath, ctx->lastErrorCode);

                    free(ctx->errorDetails.filePath);

                    break;
                }
                case ERROR_TYPE_CANNOT_READ_COM_PORT:
                case ERROR_TYPE_CANNOT_WRITE_COM_PORT:
                case ERROR_TYPE_CANNOT_WRITE_FILE: {
                    Format(errorMsg, 1024, errorMsgFormat, ctx->lastErrorCode);

                    break;
                }
                case ERROR_TYPE_PORT_NOT_SPECIFIED:
                case ERROR_TYPE_FILE_PATH_NOT_SPECIFIED:
                case ERROR_TYPE_DESTINATION_NOT_SPECIFIED:
                case ERROR_TYPE_SHA1_INPUT:
                case ERROR_TYPE_SHA1_CALCULATION: {
                    Format(errorMsg, 1024, errorMsgFormat);

                    break;
                }
                case ERROR_TYPE_CANNOT_OPEN_COM_PORT: {
                    Format(errorMsg, 1024, errorMsgFormat, ctx->errorDetails.portName, ctx->lastErrorCode);

                    free(ctx->errorDetails.portName);

                    break;
                }
                case ERROR_TYPE_BYTES_READ_MISMATCH:
                case ERROR_TYPE_BYTES_WRITTEN_MISMATCH: {
                    Format(
                        errorMsg,
                        1024,
                        errorMsgFormat,
                        ctx->errorDetails.mismatchError.expected.dword,
                        ctx->errorDetails.mismatchError.actual.dword);

                    break;
                }
                case ERROR_TYPE_SHA1_MISMATCH: {
                    Format(
                        errorMsg,
                        1024,
                        errorMsgFormat,
                        ctx->errorDetails.mismatchError.expected.wchar,
                        ctx->errorDetails.mismatchError.actual.wchar);

                    free(ctx->errorDetails.mismatchError.expected.wchar);
                    free(ctx->errorDetails.mismatchError.actual.wchar);

                    break;
                }
                case ERROR_TYPE_SERIAL_START_SIGNATURE_MISMATCH:
                case ERROR_TYPE_SERIAL_FINAL_SIGNATURE_MISMATCH: {
                    Format(
                        errorMsg,
                        1024,
                        errorMsgFormat,
                        ctx->errorDetails.mismatchError.expected.u64,
                        ctx->errorDetails.mismatchError.actual.u64);

                    break;
                }
            }

            free(ctx);

            MessageBoxW(g_mainWindow, errorMsg, errorDialogTitle, MB_ICONERROR | MB_OK);

            return 0;
        }
        case WM_INITDIALOG: {
            g_mainWindow = hwnd;

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

            int parts[2] = {MAIN_DIALOG_WIDTH / 2, -1};
            SendMessageW(g_mainWindowStatusBar, SB_SETPARTS, (WPARAM)2, (LPARAM)parts);

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

            g_portSelectComboBox = GetDlgItem(g_mainWindow, IDCOMBO_PORT);
            g_portSelectUpdateButton = GetDlgItem(g_mainWindow, IDBUTTON_UPDATE_PORT_LIST);
            g_modeChangeButtonSendMode = GetDlgItem(g_mainWindow, IDBUTTON_SEND_MODE);
            g_modeChangeButtonReceiveMode = GetDlgItem(g_mainWindow, IDBUTTON_RECEIVE_MODE);
            g_targetPathTextBox = GetDlgItem(g_mainWindow, IDEDIT_TARGET);
            g_targetBrowseButton = GetDlgItem(g_mainWindow, IDBUTTON_BROWSE_TARGET);
            g_executeButton = GetDlgItem(g_mainWindow, IDBUTTON_EXECUTE);

            SetStatusBarText(STATUS_BAR_STATUS_READY);

            UpdatePortSelectList();

            SetApplicationMode(APPLICATION_MODE_SEND_MODE);

            wchar_t currentDir[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, currentDir);

            if (g_lastReceiveModeTargetText != NULL)
            {
                free(g_lastReceiveModeTargetText);
                g_lastReceiveModeTargetText = NULL;
            }
            g_lastReceiveModeTargetText = _wcsdup(currentDir);

            return TRUE;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam))
            {
                case IDCANCEL: {
                    EndDialog(hwnd, 0);

                    return TRUE;
                }
                case IDMENUENTRY_FILE_EXIT: {
                    EndDialog(hwnd, 0);

                    return TRUE;
                }
                case IDMENUENTRY_FILE_VERSION: {
                    ShowVersion();

                    return TRUE;
                }
                case IDMENUENTRY_BAUDRATE: {
                    ShowBaudRateSettingWindow();

                    return TRUE;
                }
                case IDBUTTON_UPDATE_PORT_LIST: {
                    UpdatePortSelectList();

                    return TRUE;
                }
                case IDBUTTON_SEND_MODE: {
                    SetApplicationMode(APPLICATION_MODE_SEND_MODE);

                    return TRUE;
                }
                case IDBUTTON_RECEIVE_MODE: {
                    SetApplicationMode(APPLICATION_MODE_RECEIVE_MODE);

                    return TRUE;
                }
                case IDBUTTON_EXECUTE: {
                    switch (g_currentApplicationMode)
                    {
                        case APPLICATION_MODE_SEND_MODE: {
                            wchar_t *selectedPortName;
                            if (!GetSelectedPortName(&selectedPortName))
                            {
                                PleaseSpecifyPortError();

                                return TRUE;
                            }

                            wchar_t filePath[MAX_PATH];
                            GetTargetPath(filePath);
                            if (wcslen(filePath) == 0)
                            {
                                PleaseSpecifyFilePathError();

                                return TRUE;
                            }

                            SendFile(selectedPortName, filePath);

                            return TRUE;
                        }
                        case APPLICATION_MODE_RECEIVE_MODE: {

                            return TRUE;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}

void ShowMainWindow(void)
{
    DialogBoxW(g_mainInstance, MAKEINTRESOURCEW(IDDIALOG_MAIN_DIALOG), NULL, MainDialogDlgProc);
}

void FinaliseUI(void)
{
    FreePortSelectListPortNamePointers();

    if (g_lastSendModeTargetText != NULL)
    {
        free(g_lastSendModeTargetText);
        g_lastSendModeTargetText = NULL;
    }
    if (g_lastReceiveModeTargetText != NULL)
    {
        free(g_lastReceiveModeTargetText);
        g_lastReceiveModeTargetText = NULL;
    }
}
