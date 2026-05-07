#include <stdint.h>
#include <windows.h>

#include "Error.h"
#include "Serial.h"
#include "UI.h"
#include "Util.h"

#define BINARY_BUFFER_SIZE 4096

#define SFTT_SERIAL_START_SIGNATURE 0x4F8A2B9C1E3D7654ULL
#define SFTT_SERIAL_FINAL_SIGNATURE 0xB2E1094D6F83A57CULL

typedef enum
{
    RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE,
    RECEIVE_STAGE_RECEIVING_FILE_NAME,
    RECEIVE_STAGE_RECEIVING_BINARY,
    RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE
} ReceiveStage;

typedef BOOL(WINAPI *PCANCELIOEX)(HANDLE, LPOVERLAPPED);

ReceiveStage g_currentReceiveStage;
wchar_t *g_receiveDirectory;
uint64_t g_receivingFileSize;
uint64_t g_receivingFileNameSize;
uint64_t g_receivedBytesTotal;
wchar_t *g_receivingFileName;
HANDLE g_receivingCOMPortHandle;
HANDLE g_receivingFileHandle;
HANDLE g_receiverThreadHandle;
volatile BOOL g_isReceiving = FALSE;

HANDLE g_senderThreadHandle;
HANDLE g_sendingCOMPortHandle;
HANDLE g_sendingFileHandle;
wchar_t *g_sendingFileName;
size_t g_sendingFileSize;
size_t g_sendingFileNameSize;

DCB g_defaultDCB;

PCANCELIOEX g_cancelIOExFunc;

void InitialiseSerial(void)
{
    ZeroMemory(&g_defaultDCB, sizeof(g_defaultDCB));
    g_defaultDCB.BaudRate = CBR_9600;
    g_defaultDCB.ByteSize = 8;
    g_defaultDCB.Parity = NOPARITY;
    g_defaultDCB.StopBits = ONESTOPBIT;
    g_defaultDCB.fBinary = TRUE;
    g_defaultDCB.fOutX = FALSE;
    g_defaultDCB.fInX = FALSE;
    g_defaultDCB.fNull = FALSE;
    g_defaultDCB.fOutxCtsFlow = FALSE;
    g_defaultDCB.fRtsControl = RTS_CONTROL_ENABLE;

    union {
        FARPROC addr;
        PCANCELIOEX func;
    } converter;

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    converter.addr = GetProcAddress(kernel32, "CancelIoEx");

    if (converter.func != NULL)
    {
        g_cancelIOExFunc = converter.func;
    }
}

BOOL IsReceiving(void)
{
    return g_isReceiving;
}

BOOL OpenCOMPort(LPCWSTR portName, HANDLE *resultPtr)
{
    HANDLE hComPort = CreateFileW(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hComPort == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!SetCommState(hComPort, &g_defaultDCB))
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
            availablePorts[numOfAvailablePorts].friendlyPortName = _wcsdup(friendlyPortName);
            availablePorts[numOfAvailablePorts].portName = _wcsdup(portName);

            numOfAvailablePorts++;

            CloseHandle(hComm);
        }
    }

    *numberOfAvailablePorts = numOfAvailablePorts;
}

void StopReceiverThread(void)
{
    if (g_cancelIOExFunc != NULL)
    {
        g_cancelIOExFunc(g_receivingCOMPortHandle, NULL);
    }
    else
    {
        TerminateThread(g_receiverThreadHandle, 0);
    }
}

void StopReceiving(void)
{
    g_isReceiving = FALSE;

    if (g_receiverThreadHandle != INVALID_HANDLE_VALUE)
    {
        StopReceiverThread();

        WaitForSingleObject(g_receiverThreadHandle, INFINITE);

        CloseHandle(g_receiverThreadHandle);
        g_receiverThreadHandle = INVALID_HANDLE_VALUE;
    }

    if (g_receivingCOMPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_receivingCOMPortHandle);
        g_receivingCOMPortHandle = INVALID_HANDLE_VALUE;
    }

    if (g_receivingFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_receivingFileHandle);
        g_receivingFileHandle = INVALID_HANDLE_VALUE;
    }

    if (g_receiveDirectory != NULL)
    {
        free(g_receiveDirectory);
        g_receiveDirectory = NULL;
    }

    if (g_receivingFileName != NULL)
    {
        free(g_receivingFileName);
        g_receivingFileName = NULL;
    }
}

void CleanupCOMPort(void)
{
    PurgeComm(g_receivingCOMPortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    g_currentReceiveStage = RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE;
}

DWORD WINAPI ReceiverThread(LPVOID lpParam)
{
    (void)lpParam;

    OVERLAPPED ov = {0};
    ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    while (g_isReceiving)
    {
        DWORD dwEventMask;
        if (!WaitCommEvent(g_receivingCOMPortHandle, &dwEventMask, &ov))
        {
            continue;
        }

        if (!(dwEventMask & EV_RXCHAR))
        {
            continue;
        }

        COMSTAT comStat;
        DWORD dwErrors;
        ClearCommError(g_receivingCOMPortHandle, &dwErrors, &comStat);

        switch (g_currentReceiveStage)
        {
            case RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE: {
                if (comStat.cbInQue < (3 * sizeof(uint64_t)))
                {
                    continue;
                }

                SetStatusBarText(STATUS_BAR_STATUS_RECEIVING);
                EnableStartReceivingButton(FALSE);

                uint64_t readBuffer[3];
                DWORD bytesRead;
                if (!ReadFile(g_receivingCOMPortHandle, readBuffer, 3 * sizeof(uint64_t), &bytesRead, NULL))
                {
                    CannotReadCOMPortError();

                    goto CleanUp;
                }

                if (bytesRead != (3 * sizeof(uint64_t)))
                {
                    BytesReadMismatchError(3 * sizeof(uint64_t), bytesRead);

                    goto CleanUp;
                }

                if (readBuffer[0] != SFTT_SERIAL_START_SIGNATURE)
                {
                    SerialStartSignatureMismatchError(SFTT_SERIAL_START_SIGNATURE, readBuffer[0]);

                    goto CleanUp;
                }

                g_receivingFileSize = readBuffer[1];
                g_receivingFileNameSize = readBuffer[2];

                uint64_t totalSteps = 3 + g_receivingFileSize;
                ResetProgressBar();
                SetProgressBarRange(totalSteps);

                StepProgressBar();

                g_currentReceiveStage = RECEIVE_STAGE_RECEIVING_FILE_NAME;

                continue;
            }
            case RECEIVE_STAGE_RECEIVING_FILE_NAME: {
                if (comStat.cbInQue < g_receivingFileNameSize)
                {
                    continue;
                }

                wchar_t *readBuffer = (wchar_t *)malloc(g_receivingFileNameSize);
                DWORD bytesRead;
                if (!ReadFile(g_receivingCOMPortHandle, readBuffer, g_receivingFileNameSize, &bytesRead, NULL))
                {
                    CannotReadCOMPortError();

                    goto CleanUp;
                }

                if (bytesRead != g_receivingFileNameSize)
                {
                    BytesReadMismatchError(g_receivingFileNameSize, bytesRead);

                    goto CleanUp;
                }

                if (g_receivingFileName != NULL)
                {
                    free(g_receivingFileName);
                    g_receivingFileName = NULL;
                }
                g_receivingFileName = readBuffer;

                wchar_t receivingFilePath[MAX_PATH];
                Format(receivingFilePath, MAX_PATH, L"%ls\\%ls", g_receiveDirectory, g_receivingFileName);

                if (g_receivingFileHandle != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(g_receivingFileHandle);
                    g_receivingFileHandle = NULL;
                }

                g_receivingFileHandle = CreateFileW(
                    receivingFilePath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    0,
                    NULL);

                if (g_receivingFileHandle == INVALID_HANDLE_VALUE)
                {
                    CannotOpenFileError(receivingFilePath);

                    goto CleanUp;
                }

                StepProgressBar();

                g_receivedBytesTotal = 0;
                g_currentReceiveStage = RECEIVE_STAGE_RECEIVING_BINARY;

                continue;
            }
            case RECEIVE_STAGE_RECEIVING_BINARY: {
                if (comStat.cbInQue <= 0)
                {
                    continue;
                }

                DWORD bytesToRead = g_receivingFileSize - g_receivedBytesTotal;
                if (bytesToRead > 0)
                {
                    if (bytesToRead > BINARY_BUFFER_SIZE)
                    {
                        bytesToRead = BINARY_BUFFER_SIZE;
                    }
                    if (bytesToRead > comStat.cbInQue)
                    {
                        bytesToRead = comStat.cbInQue;
                    }

                    BYTE binBuffer[BINARY_BUFFER_SIZE];
                    DWORD bytesRead;
                    if (!ReadFile(g_receivingCOMPortHandle, binBuffer, bytesToRead, &bytesRead, NULL))
                    {
                        CannotReadCOMPortError();

                        goto CleanUp;
                    }

                    DWORD bytesWritten;
                    DWORD offset = 0;
                    while (offset < bytesRead)
                    {
                        if (!WriteFile(
                                g_receivingFileHandle,
                                binBuffer + offset,
                                bytesRead - offset,
                                &bytesWritten,
                                NULL))
                        {
                            CannotWriteFileError();

                            goto CleanUp;
                        }

                        if (bytesWritten == 0)
                        {
                            Sleep(1);

                            continue;
                        }

                        offset += bytesWritten;
                    }

                    g_receivedBytesTotal += bytesRead;
                    AddStepsToProgressBar(bytesRead);
                }

                if (g_receivedBytesTotal >= g_receivingFileSize)
                {
                    g_currentReceiveStage = RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE;
                }

                continue;
            }
            case RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE: {
                if (comStat.cbInQue < 8)
                {
                    continue;
                }

                uint64_t readBuffer[1];
                DWORD bytesRead;
                if (!ReadFile(g_receivingCOMPortHandle, readBuffer, sizeof(uint64_t), &bytesRead, NULL))
                {
                    CannotReadCOMPortError();

                    goto CleanUp;
                }

                if (bytesRead != sizeof(uint64_t))
                {
                    BytesReadMismatchError(sizeof(uint64_t), bytesRead);

                    goto CleanUp;
                }

                if (readBuffer[0] != SFTT_SERIAL_FINAL_SIGNATURE)
                {
                    SerialFinalSignatureMismatchError(SFTT_SERIAL_FINAL_SIGNATURE, readBuffer[0]);

                    goto CleanUp;
                }

                if (g_receivingFileName != NULL)
                {
                    free(g_receivingFileName);
                    g_receivingFileName = NULL;
                }

                if (g_receivingFileHandle != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(g_receivingFileHandle);
                    g_receivingFileHandle = INVALID_HANDLE_VALUE;
                }

                StepProgressBar();

                SetStatusBarText(STATUS_BAR_STATUS_READY);
                EnableStartReceivingButton(TRUE);

                CleanupCOMPort();

                continue;
            }
        }

    CleanUp:
        CleanupCOMPort();
        SetStatusBarText(STATUS_BAR_STATUS_READY);
        EnableStartReceivingButton(TRUE);
    }

    return 0;
}

BOOL StartReceiving(wchar_t *portName, wchar_t *receiveDir)
{
    if (g_receivingCOMPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_receivingCOMPortHandle);
        g_receivingCOMPortHandle = INVALID_HANDLE_VALUE;
    }

    if (!OpenCOMPort(portName, &g_receivingCOMPortHandle))
    {
        return FALSE;
    }

    SetCommMask(g_receivingCOMPortHandle, EV_RXCHAR);

    g_receiveDirectory = _wcsdup(receiveDir);
    g_currentReceiveStage = RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE;
    g_isReceiving = TRUE;

    if (g_receiverThreadHandle != INVALID_HANDLE_VALUE)
    {
        TerminateThread(g_receiverThreadHandle, 0);
        CloseHandle(g_receiverThreadHandle);
        g_receiverThreadHandle = INVALID_HANDLE_VALUE;
    }
    g_receiverThreadHandle = CreateThread(NULL, 0, ReceiverThread, NULL, 0, NULL);

    return TRUE;
}

DWORD WINAPI SenderThread(LPVOID lpParam)
{
    (void)lpParam;

    size_t totalSteps = 3 + g_sendingFileSize;
    ResetProgressBar();
    SetProgressBarRange(totalSteps);

    DWORD bytesWritten;

    uint64_t sendBuffer[3] = {
        SFTT_SERIAL_START_SIGNATURE,
        (uint64_t)g_sendingFileSize,
        (uint64_t)g_sendingFileNameSize};
    if (!WriteFile(g_sendingCOMPortHandle, sendBuffer, 3 * sizeof(uint64_t), &bytesWritten, NULL))
    {
        CannotWriteCOMPortError();

        goto CleanUp;
    }
    if (bytesWritten != (3 * sizeof(uint64_t)))
    {
        BytesWrittenMismatchError(sizeof(sendBuffer), bytesWritten);

        goto CleanUp;
    }

    StepProgressBar();

    if (!WriteFile(g_sendingCOMPortHandle, g_sendingFileName, g_sendingFileNameSize, &bytesWritten, NULL))
    {
        CannotWriteCOMPortError();

        goto CleanUp;
    }
    if (bytesWritten != g_sendingFileNameSize)
    {
        BytesWrittenMismatchError(g_sendingFileNameSize, bytesWritten);

        goto CleanUp;
    }

    StepProgressBar();

    BYTE binBuffer[BINARY_BUFFER_SIZE];
    DWORD bytesWrittenTotal = 0;
    DWORD bytesRead;
    while (ReadFile(g_sendingFileHandle, binBuffer, sizeof(binBuffer), &bytesRead, NULL) && bytesRead > 0)
    {
        DWORD bytesWrittenThisTime;
        DWORD offset = 0;
        while (offset < bytesRead)
        {
            if (!WriteFile(g_sendingCOMPortHandle, binBuffer + offset, bytesRead - offset, &bytesWrittenThisTime, NULL))
            {
                CannotWriteCOMPortError();

                goto CleanUp;
            }

            if (bytesWrittenThisTime == 0)
            {
                Sleep(1);

                continue;
            }

            AddStepsToProgressBar(bytesWrittenThisTime);

            offset += bytesWrittenThisTime;
        }

        bytesWrittenTotal += offset;
    }

    if (bytesWrittenTotal != g_sendingFileSize)
    {
        BytesWrittenMismatchError(g_sendingFileSize, bytesWrittenTotal);

        goto CleanUp;
    }

    uint64_t finalSignature = SFTT_SERIAL_FINAL_SIGNATURE;
    if (!WriteFile(g_sendingCOMPortHandle, &finalSignature, sizeof(uint64_t), &bytesWritten, NULL))
    {
        CannotWriteCOMPortError();

        goto CleanUp;
    }
    if (bytesWritten != sizeof(uint64_t))
    {
        BytesWrittenMismatchError(sizeof(uint64_t), bytesWritten);

        goto CleanUp;
    }

    StepProgressBar();

CleanUp:
    if (g_sendingCOMPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_sendingCOMPortHandle);
        g_sendingCOMPortHandle = INVALID_HANDLE_VALUE;
    }
    if (g_sendingFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_sendingFileHandle);
        g_sendingFileHandle = INVALID_HANDLE_VALUE;
    }
    if (g_sendingFileName != NULL)
    {
        free(g_sendingFileName);
        g_sendingFileName = NULL;
    }

    UIFinishSending();

    return 0;
}

void SendFile(wchar_t *portName, wchar_t *filePath)
{
    UIStartSending();

    HANDLE handleFile =
        CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handleFile == INVALID_HANDLE_VALUE)
    {
        CannotOpenFileError(filePath);

        UIFinishSending();
        return;
    }

    LARGE_INTEGER fileSize;
    fileSize.LowPart = (LONG)GetFileSize(handleFile, (DWORD *)&fileSize.HighPart);
    if (fileSize.LowPart == INVALID_FILE_SIZE)
    {
        CannotGetFileSizeError(filePath);

        CloseHandle(handleFile);

        UIFinishSending();
        return;
    }

    wchar_t *fileName = GetFileName(filePath);
    size_t fileNameSize = (wcslen(fileName) + 1) * sizeof(wchar_t);

    HANDLE hComPort;
    if (!OpenCOMPort(portName, &hComPort))
    {
        CannotOpenCOMPortError(portName);

        CloseHandle(handleFile);

        UIFinishSending();
        return;
    }

    if (g_sendingCOMPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_sendingCOMPortHandle);
        g_sendingCOMPortHandle = INVALID_HANDLE_VALUE;
    }
    g_sendingCOMPortHandle = hComPort;

    if (g_sendingFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_sendingFileHandle);
        g_sendingFileHandle = INVALID_HANDLE_VALUE;
    }
    g_sendingFileHandle = handleFile;

    if (g_sendingFileName != NULL)
    {
        free(g_sendingFileName);
        g_sendingFileName = NULL;
    }
    g_sendingFileName = _wcsdup(fileName);

    g_sendingFileNameSize = fileNameSize;
    g_sendingFileSize = fileSize.QuadPart;

    if (g_senderThreadHandle != INVALID_HANDLE_VALUE)
    {
        TerminateThread(g_senderThreadHandle, 0);
        CloseHandle(g_senderThreadHandle);
        g_senderThreadHandle = INVALID_HANDLE_VALUE;
    }
    g_senderThreadHandle = CreateThread(NULL, 0, SenderThread, NULL, 0, NULL);
}

void FinaliseSerial(void)
{
    if (g_isReceiving)
    {
        StopReceiving();
    }

    if (g_receiverThreadHandle != INVALID_HANDLE_VALUE)
    {
        TerminateThread(g_receiverThreadHandle, 0);
        CloseHandle(g_receiverThreadHandle);
        g_receiverThreadHandle = INVALID_HANDLE_VALUE;
    }

    if (g_receivingCOMPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_receivingCOMPortHandle);
        g_receivingCOMPortHandle = INVALID_HANDLE_VALUE;
    }

    if (g_receivingFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_receivingFileHandle);
        g_receivingFileHandle = INVALID_HANDLE_VALUE;
    }

    if (g_receiveDirectory != NULL)
    {
        free(g_receiveDirectory);
        g_receiveDirectory = NULL;
    }

    if (g_receivingFileName != NULL)
    {
        free(g_receivingFileName);
        g_receivingFileName = NULL;
    }

    if (g_senderThreadHandle != INVALID_HANDLE_VALUE)
    {
        TerminateThread(g_senderThreadHandle, 0);
        CloseHandle(g_senderThreadHandle);
        g_senderThreadHandle = INVALID_HANDLE_VALUE;
    }

    if (g_sendingCOMPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_sendingCOMPortHandle);
        g_sendingCOMPortHandle = INVALID_HANDLE_VALUE;
    }

    if (g_sendingFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_sendingFileHandle);
        g_sendingFileHandle = INVALID_HANDLE_VALUE;
    }

    if (g_sendingFileName != NULL)
    {
        free(g_sendingFileName);
        g_sendingFileName = NULL;
    }
}
