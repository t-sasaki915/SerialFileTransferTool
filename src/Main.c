#include <windows.h>

#include "Serial.h"
#include "UI.h"
#include "Util.h"

int main(void)
{
    InitialiseFormatFunction();
    InitialiseSerial();
    InitialiseUI();

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
