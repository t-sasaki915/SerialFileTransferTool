#ifndef SERIAL_H
#define SERIAL_H

#include <windows.h>

#define COM_PORT_TRY_MAX 20
#define BAUD_RATE_MAX 921600

typedef struct
{
    wchar_t *friendlyPortName;
    wchar_t *portName;
} AvailablePort;

void InitialiseSerial(void);

DWORD GetCurrentBaudRate(void);

void SetBaudRate(DWORD newBaudRate);

void GetAvailablePorts(AvailablePort *resultPtr, int *numberOfAvailablePorts);

BOOL IsReceiving(void);

void StopReceiving(void);

BOOL StartReceiving(wchar_t *portName, wchar_t *receiveDir);

void SendFile(wchar_t *portName, wchar_t *filePath);

void FinaliseSerial(void);

#endif
