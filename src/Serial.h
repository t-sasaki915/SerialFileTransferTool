#ifndef SERIAL_H
#define SERIAL_H

#include <windows.h>

#define COM_PORT_TRY_MAX 20

typedef struct
{
    wchar_t *friendlyPortName;
    wchar_t *portName;
} AvailablePort;

void InitialiseSerial(void);

void GetAvailablePorts(AvailablePort *resultPtr, int *numberOfAvailablePorts);

BOOL IsReceiving(void);

void StopReceiving(void);

BOOL StartReceiving(LPCWSTR portName);

void SendFile(wchar_t *portName, wchar_t *filePath);

void FinaliseSerial(void);

#endif
