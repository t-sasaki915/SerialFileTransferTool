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
#define WM_SFTT_ENABLE_CONTROL WM_USER + 2

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

static wchar_t *g_lastSendModeTargetText = NULL;
static wchar_t *g_lastReceiveModeTargetText = NULL;

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
            LoadStringW(g_mainInstance, IDSTRING_EXECUTE_BUTTON_LABEL_RECEIVE_MODE_START, newExecuteButtonText, 256);

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
    ofn.lpstrFilter = L"All Files\0*.*";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    return GetOpenFileNameW(&ofn);
}

BOOL BrowseReceiveDirectory(wchar_t *resultPtr)
{
    wchar_t title[512];
    LoadStringW(g_mainInstance, IDSTRING_RECEIVE_DESTINATION_BROWSE_DIALOG_TITLE, title, 512);

    BROWSEINFOW bi = {0};
    bi.hwndOwner = g_mainWindow;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi.lpszTitle = title;

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

void UpdateTargetPath(wchar_t *newPath)
{
    SetWindowTextW(g_targetPathTextBox, newPath);
}

void GetTargetPath(wchar_t *resultPtr)
{
    GetWindowTextW(g_targetPathTextBox, resultPtr, MAX_PATH);
}

void RequestErrorDialog(ErrorContext *errorContext)
{
    SendMessageW(g_mainWindow, WM_SFTT_SHOW_ERROR_DIALOG, (WPARAM)errorContext, (LPARAM)0);
}

void EnableBaudRateSettingButton(BOOL enable)
{
    EnableMenuItem(GetMenu(g_mainWindow), IDMENUENTRY_BAUDRATE, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED));
    DrawMenuBar(g_mainWindow);
}

void EnableReceiveModeControls(BOOL enable)
{
    wchar_t newExecuteButtonLabel[128];
    LoadStringW(
        g_mainInstance,
        enable ? IDSTRING_EXECUTE_BUTTON_LABEL_RECEIVE_MODE_START : IDSTRING_EXECUTE_BUTTON_LABEL_RECEIVE_MODE_STOP,
        newExecuteButtonLabel,
        128);
    SetWindowTextW(g_executeButton, newExecuteButtonLabel);

    EnableWindow(g_portSelectComboBox, enable);
    EnableWindow(g_portSelectUpdateButton, enable);
    EnableWindow(g_modeChangeButtonSendMode, enable);
    EnableWindow(g_targetBrowseButton, enable);
    EnableWindow(g_targetPathTextBox, enable);

    EnableBaudRateSettingButton(enable);
}

void SetStatusBarText(StatusBarStatus status)
{
    wchar_t newStatusBarText[64];
    LoadStringW(g_mainInstance, status, newStatusBarText, 64);

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

void EnableSendModeControls(BOOL enable)
{
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_portSelectComboBox, (LPARAM)enable);
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_portSelectUpdateButton, (LPARAM)enable);
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_modeChangeButtonReceiveMode, (LPARAM)enable);
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_targetPathTextBox, (LPARAM)enable);
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_targetBrowseButton, (LPARAM)enable);
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_executeButton, (LPARAM)enable);

    EnableBaudRateSettingButton(enable);
}

void EnableExecuteButton(BOOL enable)
{
    SendMessageW(g_mainWindow, WM_SFTT_ENABLE_CONTROL, (WPARAM)g_executeButton, (LPARAM)enable);
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

INT_PTR CALLBACK BaudRateSettingDialogDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;

    switch (wMsg)
    {
        case WM_INITDIALOG: {
            wchar_t textBoxInitContent[16];
            Format(textBoxInitContent, 16, L"%lu", GetCurrentBaudRate());
            SetWindowTextW(GetDlgItem(hwnd, IDEDIT_BRS_NEW_BAUD_RATE), textBoxInitContent);

            return TRUE;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam))
            {
                case IDCANCEL: {
                    EndDialog(hwnd, 0);

                    return TRUE;
                }
                case IDBUTTON_BRS_CANCEL: {
                    EndDialog(hwnd, 0);

                    return TRUE;
                }
                case IDBUTTON_BRS_OK: {
                    BOOL result;
                    UINT newBaudRate = GetDlgItemInt(hwnd, IDEDIT_BRS_NEW_BAUD_RATE, &result, FALSE);

                    if (!result)
                    {
                        wchar_t errorDialogTitle[64];
                        LoadStringW(g_mainInstance, IDSTRING_BRS_ERROR_DIALOG_TITLE, errorDialogTitle, 64);
                        wchar_t errorMsg[128];
                        LoadStringW(g_mainInstance, IDSTRING_BRS_BAUD_RATE_EMPTY_ERROR, errorMsg, 128);

                        MessageBoxW(NULL, errorMsg, errorDialogTitle, MB_ICONERROR | MB_OK);

                        return TRUE;
                    }

                    if (newBaudRate <= 0 || newBaudRate > BAUD_RATE_MAX)
                    {
                        wchar_t errorDialogTitle[64];
                        LoadStringW(g_mainInstance, IDSTRING_BRS_ERROR_DIALOG_TITLE, errorDialogTitle, 64);
                        wchar_t errorMsg[128];
                        LoadStringW(g_mainInstance, IDSTRING_BRS_BAUD_RATE_OUT_OF_RANGE_ERROR, errorMsg, 128);

                        MessageBoxW(NULL, errorMsg, errorDialogTitle, MB_ICONERROR | MB_OK);

                        return TRUE;
                    }

                    SetBaudRate((DWORD)newBaudRate);

                    EndDialog(hwnd, 0);

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

void ShowBaudRateSettingWindow(void)
{
    DialogBoxW(
        g_mainInstance,
        MAKEINTRESOURCEW(IDDIALOG_BAUD_RATE_SETTING),
        g_mainWindow,
        BaudRateSettingDialogDlgProc);
}

INT_PTR CALLBACK MainDialogDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;

    switch (wMsg)
    {
        case WM_SFTT_ENABLE_CONTROL: {
            EnableWindow((HWND)wParam, (BOOL)lParam);

            return 0;
        }
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
                case IDBUTTON_BROWSE_TARGET: {
                    wchar_t newPath[MAX_PATH] = {0};

                    switch (g_currentApplicationMode)
                    {
                        case APPLICATION_MODE_SEND_MODE: {
                            if (!BrowseFileToSend(newPath))
                            {
                                goto Return;
                            }

                            break;
                        }
                        case APPLICATION_MODE_RECEIVE_MODE: {
                            if (!BrowseReceiveDirectory(newPath))
                            {
                                goto Return;
                            }

                            break;
                        }
                    }

                    UpdateTargetPath(newPath);

                Return:
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
                            if (IsReceiving())
                            {
                                StopReceiving();
                                EnableReceiveModeControls(TRUE);
                            }
                            else
                            {
                                wchar_t *selectedPortName;
                                if (!GetSelectedPortName(&selectedPortName))
                                {
                                    PleaseSpecifyPortError();

                                    return TRUE;
                                }

                                wchar_t receiveDir[MAX_PATH];
                                GetTargetPath(receiveDir);
                                if (wcslen(receiveDir) == 0)
                                {
                                    PleaseSpecifyDirectoryError();

                                    return TRUE;
                                }

                                if (StartReceiving(selectedPortName, receiveDir))
                                {
                                    EnableReceiveModeControls(FALSE);
                                }
                                else
                                {
                                    StopReceiving();
                                    EnableReceiveModeControls(TRUE);

                                    CannotOpenCOMPortError(selectedPortName);

                                    return TRUE;
                                }
                            }

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
