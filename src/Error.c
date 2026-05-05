#include <windows.h>

#include "UI.h"
#include "Util.h"

#define CANNOT_OPEN_FILE_ERROR_MSG L"Cannot open the file '%ls': %lu."
#define CANNOT_OPEN_FILE_ERROR_MSG_LENGTH 350

#define CANNOT_GET_FILE_SIZE_ERROR_MSG L"Cannot get the size of the file '%ls': %lu."
#define CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH 400

#define PLEASE_SPECIFY_PORT_ERROR_MSG L"Please specify a port."

#define PLEASE_SPECIFY_FILE_PATH_ERROR_MSG L"Please specify a filepath."

#define CANNOT_OPEN_COM_PORT_ERROR_MSG L"Cannot open the COM port '%ls': %lu."
#define CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH 100

void CannotOpenCOMPortError(wchar_t *portName)
{
    wchar_t msg[CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH];
    Format(msg, CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH, CANNOT_OPEN_COM_PORT_ERROR_MSG, portName, GetLastError());

    RequestErrorDialog(msg);
}

void PleaseSpecifyPortError(void)
{
    RequestErrorDialog(PLEASE_SPECIFY_PORT_ERROR_MSG);
}

void PleaseSpecifyFilePathError(void)
{
    RequestErrorDialog(PLEASE_SPECIFY_FILE_PATH_ERROR_MSG);
}
