#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <windows.h>

#include "Error.h"

typedef enum
{
    APPLICATION_MODE_SEND_MODE,
    APPLICATION_MODE_RECEIVE_MODE
} ApplicationMode;

typedef enum
{
    STATUS_BAR_STATUS_READY = IDSTRING_STATUSBAR_TEXT_READY,
    STATUS_BAR_STATUS_SENDING = IDSTRING_STATUSBAR_TEXT_SENDING,
    STATUS_BAR_STATUS_RECEIVING = IDSTRING_STATUSBAR_TEXT_RECEIVING
} StatusBarStatus;

void InitialiseUI(void);

void SetApplicationMode(ApplicationMode appMode);

void ShowMainWindow(void);

void PaintMainWindow(void);

void EraseWindowBackground(HWND hwnd, HDC hdc);

void UpdatePortSelectList(void);

BOOL GetSelectedPortName(wchar_t **resultPtr);

BOOL BrowseFileToSend(wchar_t *resultPtr);

void GetTargetPath(wchar_t *resultPtr);

void RequestErrorDialog(ErrorContext *errorContext);

void EnableReceiveModeControls(BOOL enable);

void SetStatusBarText(StatusBarStatus status);

void SetProgressBarRange(uint32_t max);

void EnableSetModeControls(BOOL enable);

void StepProgressBar(void);

void AddStepsToProgressBar(uint32_t steps);

void ResetProgressBar(void);

void UIStartSending(void);

void UIFinishSending(void);

void EnableExecuteButton(BOOL enable);

void EnableBaudRateSettingButton(BOOL enable);

void EnableBaudRateSettingButton(BOOL enable);

void FinaliseUI(void);

#endif
