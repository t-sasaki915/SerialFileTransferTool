#ifndef UI_H
#define UI_H

#include <windows.h>

typedef enum
{
    APPLICATION_MODE_SEND_MODE,
    APPLICATION_MODE_RECEIVE_MODE
} ApplicationMode;

typedef enum
{
    STATUS_BAR_STATUS_READY,
    STATUS_BAR_STATUS_SENDING,
    STATUS_BAR_STATUS_RECEIVING
} StatusBarStatus;

typedef void (*P_ONPORTSELECTUPDATEBUTTON)(void);
typedef void (*P_ONMODECHANGEBUTTONSENDMODE)(void);
typedef void (*P_ONMODECHANGEBUTTONRECEIVEMODE)(void);
typedef void (*P_ONSTARTRECEIVINGBUTTON)(void);
typedef void (*P_ONSENDFILEPATHBROWSEBUTTON)(void);
typedef void (*P_ONSENDFILEBUTTON)(void);

typedef struct
{
    P_ONPORTSELECTUPDATEBUTTON onPortSelectUpdateButton;
    P_ONMODECHANGEBUTTONSENDMODE onModeChangeButtonSendMode;
    P_ONMODECHANGEBUTTONRECEIVEMODE onModeChangeButtonReceiveMode;
    P_ONSTARTRECEIVINGBUTTON onStartReceivingButton;
    P_ONSENDFILEPATHBROWSEBUTTON onSendFilePathBrowseButton;
    P_ONSENDFILEBUTTON onSendFileButton;
} LogicSet;

void InitialiseUI(LogicSet mainLogicSet);

void SetApplicationMode(ApplicationMode appMode);

void ShowMainWindow(void);

void PaintMainWindow(void);

void EraseWindowBackground(HWND hwnd, HDC hdc);

void UpdatePortSelectList(void);

BOOL GetSelectedPortName(wchar_t **resultPtr);

BOOL BrowseFileToSend(wchar_t *resultPtr);

void UpdateSendFilePathTextbox(wchar_t *filePath);

void GetSendFilePath(wchar_t *resultPtr);

void RequestErrorDialog(wchar_t *msg);

void UIStopReceiving(void);

void UIStartReceiving(void);

void SetStatusBarText(StatusBarStatus status);

void EnableSetModeControls(BOOL enable);

void FinaliseUI(void);

#endif
