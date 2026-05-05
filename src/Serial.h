#ifndef SERIAL_H
#define SERIAL_H

#include <windows.h>

#define SFTT_SERIAL_START_SIGNATURE 0x4F8A2B9C1E3D7654ULL
#define SFTT_SERIAL_FINAL_SIGNATURE 0xB2E1094D6F83A57CULL

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

void StartReceiving(LPCWSTR portName);

void SendFile(wchar_t* portName, wchar_t* filePath);

void FinaliseSerial(void);

#endif
