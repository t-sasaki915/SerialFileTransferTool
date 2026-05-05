#include <windows.h>

#include "Error.h"
#include "Serial.h"
#include "UI.h"
#include "Util.h"

void OnModeChangeButtonSendMode(void)
{
    SetApplicationMode(APPLICATION_MODE_SEND_MODE);
}

void OnModeChangeButtonReceiveMode(void)
{
    SetApplicationMode(APPLICATION_MODE_RECEIVE_MODE);
}

void OnStartReceivingButton(void)
{
    if (IsReceiving())
    {
        StopReceiving();
    }
    else
    {
        wchar_t *selectedPortName;
        if (!GetSelectedPortName(&selectedPortName))
        {
            PleaseSpecifyPortError();

            return;
        }

        if (!StartReceiving(selectedPortName))
        {
            CannotOpenCOMPortError(selectedPortName);

            return;
        }
    }
}

void OnSendFilePathBrowseButton(void)
{
    wchar_t filePath[MAX_PATH_LENGTH] = L"";

    if (BrowseFileToSend(filePath))
    {
        UpdateSendFilePathTextbox(filePath);
    }
}

void OnSendFileButton(void)
{
    wchar_t *selectedPortName;
    if (!GetSelectedPortName(&selectedPortName))
    {
        PleaseSpecifyPortError();

        return;
    }

    wchar_t filePath[MAX_PATH_LENGTH];
    GetSendFilePath(filePath);
    if (wcslen(filePath) == 0)
    {
        PleaseSpecifyFilePathError();

        return;
    }

    EnableSetModeControls(FALSE);

    SendFile(selectedPortName, filePath);

    EnableSetModeControls(TRUE);
}

int main(void)
{
    LogicSet logicSet;
    ZeroMemory(&logicSet, sizeof(logicSet));
    logicSet.onPortSelectUpdateButton = UpdatePortSelectList;
    logicSet.onModeChangeButtonSendMode = OnModeChangeButtonSendMode;
    logicSet.onModeChangeButtonReceiveMode = OnModeChangeButtonReceiveMode;
    logicSet.onStartReceivingButton = OnStartReceivingButton;
    logicSet.onSendFilePathBrowseButton = OnSendFilePathBrowseButton;
    logicSet.onSendFileButton = OnSendFileButton;

    InitialiseSerial();
    InitialiseUI(logicSet);

    ShowMainWindow();

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

    FinaliseSerial();
    FinaliseUI();

    return 0;
}
