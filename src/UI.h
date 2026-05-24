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

void InitialiseUI(HINSTANCE hInstance);

void ShowMainWindow(void);

void UpdatePortSelectList(void);

void GetTargetPath(wchar_t *resultPtr);

void RequestErrorDialog(ErrorContext *errorContext);

void SetStatusBarText(StatusBarStatus status);

void EnableReceiveModeControls(BOOL enable);

void EnableSendModeControls(BOOL enable);

void SetProgressBarRange(uint32_t max);

void StepProgressBar(void);

void AddStepsToProgressBar(uint32_t steps);

void ResetProgressBar(void);

void EnableExecuteButton(BOOL enable);

void EnableBaudRateSettingButton(BOOL enable);

void FinaliseUI(void);

#endif
