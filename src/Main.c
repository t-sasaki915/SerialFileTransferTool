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

    FinaliseSerial();
    FinaliseUI();

    return 0;
}
