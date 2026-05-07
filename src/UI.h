#ifndef UI_H
#define UI_H

#include <stdint.h>
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

void InitialiseUI(void);

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

void SetProgressBarRange(uint32_t max);

void EnableSetModeControls(BOOL enable);

void StepProgressBar(void);

void AddStepsToProgressBar(uint32_t steps);

void ResetProgressBar(void);

void UIStartSending(void);

void UIFinishSending(void);

void EnableStartReceivingButton(BOOL enable);

void EnableBaudRateSettingButton(BOOL enable);

void FinaliseUI(void);

#endif
