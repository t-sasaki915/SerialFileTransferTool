#include <windows.h>

#include "Serial.h"
#include "UI.h"
#include "Util.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    InitialiseFormatFunction();
    InitialiseSerial();
    InitialiseUI(hInstance);

    ShowMainWindow();

    FinaliseSerial();
    FinaliseUI();

    return 0;
}
