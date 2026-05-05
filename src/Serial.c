#include <stdint.h>
#include <windows.h>

#include "Serial.h"
#include "Util.h"

typedef enum
{
    RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE,
    RECEIVE_STAGE_RECEIVING_FILE_NAME,
    RECEIVE_STAGE_RECEIVING_BINARY,
    RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE
} ReceiveStage;

typedef BOOL(WINAPI *PCANCELIOEX)(HANDLE, LPOVERLAPPED);

ReceiveStage CURRENT_RECEIVE_STAGE;
uint64_t RECEIVING_FILE_SIZE;
uint64_t RECEIVING_FILE_NAME_SIZE;
wchar_t *RECEIVING_FILE_NAME;
HANDLE RECEIVING_COM_PORT_HANDLE;
HANDLE RECEIVER_THREAD_HANDLE;
volatile BOOL IS_RECEIVING = FALSE;

DCB DEFAULT_DCB;

PCANCELIOEX CANCEL_IO_EX_FUNC;
BOOL CANCEL_IO_EX_FUNC_IS_SET = FALSE;

void InitialiseSerial(void)
{
    ZeroMemory(&DEFAULT_DCB, sizeof(DEFAULT_DCB));
    DEFAULT_DCB.fBinary = TRUE;
    DEFAULT_DCB.fOutX = FALSE;
    DEFAULT_DCB.fInX = FALSE;
    DEFAULT_DCB.fNull = FALSE;
}

BOOL IsReceiving(void)
{
    return IS_RECEIVING;
}

BOOL OpenCOMPort(LPCWSTR portName, HANDLE *resultPtr)
{
    HANDLE hComPort = CreateFileW(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hComPort == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    *resultPtr = hComPort;

    return TRUE;
}

void GetAvailablePorts(AvailablePort *availablePorts, int *numberOfAvailablePorts)
{
    int numOfAvailablePorts = 0;

    for (int i = 1; i <= COM_PORT_TRY_MAX; i++)
    {
        wchar_t friendlyPortName[10];
        Format(friendlyPortName, 10, L"COM%d", i);
        wchar_t portName[20];
        Format(portName, 20, L"\\\\.\\%ls", friendlyPortName);

        HANDLE hComm;
        if (OpenCOMPort(portName, &hComm))
        {
            availablePorts[numOfAvailablePorts].friendlyPortName = friendlyPortName;
            availablePorts[numOfAvailablePorts].portName = portName;

            numOfAvailablePorts++;

            CloseHandle(hComm);
        }
    }

    *numberOfAvailablePorts = numOfAvailablePorts;
}

void StopReceiverThread(void)
{
    if (!CANCEL_IO_EX_FUNC_IS_SET)
    {
        union {
            FARPROC addr;
            PCANCELIOEX func;
        } converter;

        HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
        converter.addr = GetProcAddress(kernel32, "CancelIoEx");

        CANCEL_IO_EX_FUNC = converter.func;

        CANCEL_IO_EX_FUNC_IS_SET = TRUE;
    }

    if (CANCEL_IO_EX_FUNC != NULL)
    {
        CANCEL_IO_EX_FUNC(RECEIVING_COM_PORT_HANDLE, NULL);
    }
    else
    {
        TerminateThread(RECEIVER_THREAD_HANDLE, 0);
    }
}

void StopReceiving(void)
{
    IS_RECEIVING = FALSE;

    if (RECEIVER_THREAD_HANDLE != INVALID_HANDLE_VALUE)
    {
        StopReceiverThread();

        WaitForSingleObject(RECEIVER_THREAD_HANDLE, INFINITE);

        CloseHandle(RECEIVER_THREAD_HANDLE);
        RECEIVER_THREAD_HANDLE = INVALID_HANDLE_VALUE;
    }

    if (RECEIVING_COM_PORT_HANDLE != INVALID_HANDLE_VALUE)
    {
        CloseHandle(RECEIVING_COM_PORT_HANDLE);
        RECEIVING_COM_PORT_HANDLE = INVALID_HANDLE_VALUE;
    }
}

DWORD WINAPI ReceiverThread(LPVOID lpParam)
{
    (void)lpParam;

    OVERLAPPED ov;
    ZeroMemory(&ov, sizeof(ov));
    ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    while (IS_RECEIVING)
    {
        DWORD dwEventMask;
        if (WaitCommEvent(RECEIVING_COM_PORT_HANDLE, &dwEventMask, &ov))
        {
            if (dwEventMask & EV_RXCHAR)
            {
                COMSTAT comStat;
                DWORD dwErrors;
                ClearCommError(RECEIVING_COM_PORT_HANDLE, &dwErrors, &comStat);

                switch (CURRENT_RECEIVE_STAGE)
                {
                    case RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE: {
                        if (comStat.cbInQue >= 24)
                        {
                            uint64_t readBuffer[3];
                            DWORD bytesRead;
                            if (!ReadFile(RECEIVING_COM_PORT_HANDLE, readBuffer, 24, &bytesRead, NULL))
                            {
                                // TODO ERROR
                                return 0;
                            }

                            if (bytesRead != 24)
                            {
                                // TODO ERROR
                                return 0;
                            }

                            if (readBuffer[0] != SFTT_SERIAL_START_SIGNATURE)
                            {
                                // TODO ERROR
                                return 0;
                            }

                            RECEIVING_FILE_SIZE = readBuffer[1];
                            RECEIVING_FILE_NAME_SIZE = readBuffer[2];

                            CURRENT_RECEIVE_STAGE = RECEIVE_STAGE_RECEIVING_FILE_NAME;
                        }

                        return 0;
                    }
                    case RECEIVE_STAGE_RECEIVING_FILE_NAME: {
                        if (comStat.cbInQue >= RECEIVING_FILE_NAME_SIZE)
                        {
                            wchar_t *readBuffer = (wchar_t *)malloc(RECEIVING_FILE_NAME_SIZE);
                            DWORD bytesRead;
                            if (!ReadFile(
                                    RECEIVING_COM_PORT_HANDLE,
                                    readBuffer,
                                    RECEIVING_FILE_NAME_SIZE,
                                    &bytesRead,
                                    NULL))
                            {
                                // TODO ERROR
                                return 0;
                            }

                            if (bytesRead != RECEIVING_FILE_NAME_SIZE)
                            {
                                // TODO ERROR
                                return 0;
                            }

                            RECEIVING_FILE_NAME = readBuffer;

                            CURRENT_RECEIVE_STAGE = RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE;
                        }

                        return 0;
                    }
                    case RECEIVE_STAGE_RECEIVING_BINARY: {
                        return 0;
                    }
                    case RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE: {
                        if (comStat.cbInQue >= 8)
                        {
                            uint64_t readBuffer[1];
                            DWORD bytesRead;
                            if (!ReadFile(RECEIVING_COM_PORT_HANDLE, readBuffer, 8, &bytesRead, NULL))
                            {
                                // TODO ERROR
                                return 0;
                            }

                            if (readBuffer[0] != SFTT_SERIAL_FINAL_SIGNATURE)
                            {
                                // TODO ERROR
                                return 0;
                            }

                            // TODO
                            CURRENT_RECEIVE_STAGE = RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE;
                        }

                        return 0;
                    }
                }
            }
        }
    }

    return 0;
}

BOOL StartReceiving(LPCWSTR portName)
{
    if (!OpenCOMPort(portName, &RECEIVING_COM_PORT_HANDLE))
    {
        StopReceiving();

        return FALSE;
    }

    CURRENT_RECEIVE_STAGE = RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE;

    SetCommState(RECEIVING_COM_PORT_HANDLE, &DEFAULT_DCB);
    SetCommMask(RECEIVING_COM_PORT_HANDLE, EV_RXCHAR);

    RECEIVER_THREAD_HANDLE = CreateThread(NULL, 0, ReceiverThread, NULL, 0, NULL);

    IS_RECEIVING = TRUE;

    return TRUE;
}

void SendFile(wchar_t *portName, wchar_t *filePath)
{
    HANDLE handleFile =
        CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handleFile == INVALID_HANDLE_VALUE)
    {
        // TODO ERROR
        return;
    }

    LARGE_INTEGER fileSize;
    fileSize.LowPart = (LONG)GetFileSize(handleFile, (DWORD *)&fileSize.HighPart);
    if (fileSize.LowPart == INVALID_FILE_SIZE)
    {
        // TODO ERROR

        CloseHandle(handleFile);
        return;
    }

    wchar_t *fileName = GetFileName(filePath);
    size_t fileNameSize = (wcslen(fileName) + 1) * sizeof(wchar_t);

    HANDLE hComPort;
    if (!OpenCOMPort(portName, &hComPort))
    {
        // TODO ERROR

        CloseHandle(handleFile);
        return;
    }

    DWORD bytesWritten;

    uint64_t sendBuffer[3] = {SFTT_SERIAL_START_SIGNATURE, (uint64_t)fileSize.QuadPart, (uint64_t)fileNameSize};
    if (!WriteFile(hComPort, sendBuffer, sizeof(sendBuffer), &bytesWritten, NULL))
    {
        // TODO ERROR

        goto CleanUp;
    }
    if (bytesWritten != sizeof(sendBuffer))
    {
        // TODO ERROR

        goto CleanUp;
    }

    if (!WriteFile(hComPort, fileName, fileNameSize, &bytesWritten, NULL))
    {
        // TODO ERROR

        goto CleanUp;
    }
    if (bytesWritten != fileNameSize)
    {
        // TODO ERROR

        goto CleanUp;
    }

    BYTE binBuffer[4096];
    DWORD bytesRead;
    while (ReadFile(handleFile, binBuffer, sizeof(binBuffer), &bytesRead, NULL) && bytesRead > 0)
    {
        if (!WriteFile(hComPort, binBuffer, bytesRead, &bytesWritten, NULL))
        {
            // TODO ERROR

            goto CleanUp;
        }
        if (bytesWritten != bytesRead)
        {
            // TODO ERROR

            goto CleanUp;
        }
    }

    uint64_t sendBuffer2[1] = {SFTT_SERIAL_FINAL_SIGNATURE};
    if (!WriteFile(hComPort, sendBuffer2, sizeof(sendBuffer2), &bytesWritten, NULL))
    {
        // TODO ERROR

        goto CleanUp;
    }
    if (bytesWritten != sizeof(sendBuffer2))
    {
        // TODO ERROR

        goto CleanUp;
    }

CleanUp:
    if (hComPort != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hComPort);
    }
    if (handleFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(handleFile);
    }
}

void FinaliseSerial(void)
{
    if (IS_RECEIVING)
    {
        StopReceiving();
    }
}
