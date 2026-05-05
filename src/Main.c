#include <inttypes.h>
#include <stdint.h>
#include <windows.h>

#include "Serial.h"
#include "UI.h"
#include "Util.h"


#define WM_SFTT_TEST WM_USER + 1

#define CANNOT_OPEN_FILE_ERROR_TITLE L"SFTT"
#define CANNOT_OPEN_FILE_ERROR_MSG L"Cannot open the file '%ls': %lu."
#define CANNOT_OPEN_FILE_ERROR_MSG_LENGTH 350

#define CANNOT_GET_FILE_SIZE_ERROR_TITLE L"SFTT"
#define CANNOT_GET_FILE_SIZE_ERROR_MSG L"Cannot get the size of the file '%ls': %lu."
#define CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH 400

#define PLEASE_SPECIFY_PORT_ERROR_TITLE L"SFTT"
#define PLEASE_SPECIFY_PORT_ERROR_MSG L"Please specify the port."

#define CANNOT_OPEN_COM_PORT_ERROR_TITLE L"SFTT"
#define CANNOT_OPEN_COM_PORT_ERROR_MSG L"Cannot open the COM port '%ls': %lu."
#define CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH 100

LRESULT CALLBACK MainWindowWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        /*case WM_SFTT_TEST: {
            wchar_t msg[1000];
            Format(
                msg,
                1000,
                L"Final Signature Matched.\nFile Size: %" PRId64 "\nFile Name Size: %" PRId64 "\nFile Name: %ls",
                RECEIVING_FILE_SIZE,
                RECEIVING_FILE_NAME_SIZE,
                RECEIVING_FILE_NAME);

            MessageBoxW(MAIN_WINDOW, msg, L"", MB_OK);

            free(RECEIVING_FILE_NAME);

            return 0;
            }*/
        case WM_DESTROY: {
            PostQuitMessage(0);

            return 0;
        }
        case WM_PAINT: {
            PaintMainWindow();

            return 0;
        }
        case WM_ERASEBKGND: {
            EraseWindowBackground(hwnd, (HDC)wParam);

            return 1;
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
                    }
                    else
                    {
                        wchar_t *selectedPortName;
                        if (!GetSelectedPortName(&selectedPortName))
                        {
                            // TODO ERROR

                            return 0;
                        }

                        StartReceiving(selectedPortName);
                    }

                    return 0;
                }
                case SEND_FILE_PATH_BROWSE_BUTTON_ID: {
                    wchar_t filePath[MAX_PATH_LENGTH] = L"";

                    if (BrowseFileToSend(filePath))
                    {
                        UpdateSendFilePathTextbox(filePath);
                    }

                    return 0;
                }
                case SEND_FILE_BUTTON_ID: {
                    EnableSetModeControls(FALSE);

                    wchar_t *selectedPortName;
                    if (!GetSelectedPortName(&selectedPortName))
                    {
                        // TODO ERROR

                        return 0;
                    }

                    wchar_t filePath[MAX_PATH_LENGTH];
                    GetSendFilePath(filePath);

                    SendFile(selectedPortName, filePath);

                    EnableSetModeControls(TRUE);

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

int main(void)
{
    InitialiseSerial();
    InitialiseUI(MainWindowWndProc);

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
