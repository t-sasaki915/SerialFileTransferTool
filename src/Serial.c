#include <stdint.h>
#include <windows.h>

#include "Error.h"
#include "Serial.h"
#include "UI.h"
#include "Util.h"

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
uint64_t g_receivingFileSize;
uint64_t g_receivingFileNameSize;
wchar_t *g_receivingFileName;
HANDLE g_receivingCOMPortHandle;
HANDLE g_receiverThreadHandle;
volatile BOOL g_isReceiving = FALSE;

HANDLE SENDER_THREAD_HANDLE;
HANDLE SENDING_COM_PORT_HANDLE;
HANDLE SENDING_FILE_HANDLE;
wchar_t *SENDING_FILE_NAME;
size_t SENDING_FILE_SIZE;
size_t SENDING_FILE_NAME_SIZE;

DCB DEFAULT_DCB;

PCANCELIOEX CANCEL_IO_EX_FUNC;
BOOL CANCEL_IO_EX_FUNC_IS_SET = FALSE;

void InitialiseSerial(void)
{
    ZeroMemory(&DEFAULT_DCB, sizeof(DEFAULT_DCB));
    DEFAULT_DCB.BaudRate = CBR_9600;
    DEFAULT_DCB.ByteSize = 8;
    DEFAULT_DCB.Parity = NOPARITY;
    DEFAULT_DCB.StopBits = ONESTOPBIT;
    DEFAULT_DCB.fBinary = TRUE;
    DEFAULT_DCB.fOutX = FALSE;
    DEFAULT_DCB.fInX = FALSE;
    DEFAULT_DCB.fNull = FALSE;
    DEFAULT_DCB.fOutxCtsFlow = FALSE;
    DEFAULT_DCB.fRtsControl = RTS_CONTROL_ENABLE;
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

    if (!SetCommState(hComPort, &DEFAULT_DCB))
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
        CANCEL_IO_EX_FUNC(g_receivingCOMPortHandle, NULL);
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
}

void CleanupCOMPort(void)
{
    PurgeComm(g_receivingCOMPortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    g_currentReceiveStage = RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE;
}

DWORD WINAPI ReceiverThread(LPVOID lpParam)
{
    (void)lpParam;

    OVERLAPPED ov;
    ZeroMemory(&ov, sizeof(ov));
    ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    while (g_isReceiving)
    {
        DWORD dwEventMask;
        if (WaitCommEvent(g_receivingCOMPortHandle, &dwEventMask, &ov))
        {
            if (dwEventMask & EV_RXCHAR)
            {
                COMSTAT comStat;
                DWORD dwErrors;
                ClearCommError(g_receivingCOMPortHandle, &dwErrors, &comStat);

                switch (g_currentReceiveStage)
                {
                    case RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE: {
                        if (comStat.cbInQue >= (3 * sizeof(uint64_t)))
                        {
                            uint64_t readBuffer[3];
                            DWORD bytesRead;
                            if (!ReadFile(g_receivingCOMPortHandle, readBuffer, 3 * sizeof(uint64_t), &bytesRead, NULL))
                            {
                                CleanupCOMPort();
                                CannotReadCOMPortError();

                                continue;
                            }

                            if (bytesRead != (3 * sizeof(uint64_t)))
                            {
                                CleanupCOMPort();
                                BytesReadMismatchError(3 * sizeof(uint64_t), bytesRead);

                                continue;
                            }

                            if (readBuffer[0] != SFTT_SERIAL_START_SIGNATURE)
                            {
                                CleanupCOMPort();
                                SerialStartSignatureMismatchError(SFTT_SERIAL_START_SIGNATURE, readBuffer[0]);

                                continue;
                            }

                            g_receivingFileSize = readBuffer[1];
                            g_receivingFileNameSize = readBuffer[2];

                            g_currentReceiveStage = RECEIVE_STAGE_RECEIVING_FILE_NAME;
                        }

                        continue;
                    }
                    case RECEIVE_STAGE_RECEIVING_FILE_NAME: {
                        if (comStat.cbInQue >= g_receivingFileNameSize)
                        {
                            wchar_t *readBuffer = (wchar_t *)malloc(g_receivingFileNameSize);
                            DWORD bytesRead;
                            if (!ReadFile(
                                    g_receivingCOMPortHandle,
                                    readBuffer,
                                    g_receivingFileNameSize,
                                    &bytesRead,
                                    NULL))
                            {
                                CleanupCOMPort();
                                CannotReadCOMPortError();

                                continue;
                            }

                            if (bytesRead != g_receivingFileNameSize)
                            {
                                CleanupCOMPort();
                                BytesReadMismatchError(g_receivingFileNameSize, bytesRead);

                                continue;
                            }

                            g_receivingFileName = readBuffer;

                            g_currentReceiveStage = RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE;
                        }

                        continue;
                    }
                    case RECEIVE_STAGE_RECEIVING_BINARY: {
                        continue;
                    }
                    case RECEIVE_STAGE_RECEIVING_FINAL_SIGNATURE: {
                        if (comStat.cbInQue >= 8)
                        {
                            uint64_t readBuffer[1];
                            DWORD bytesRead;
                            if (!ReadFile(g_receivingCOMPortHandle, readBuffer, 8, &bytesRead, NULL))
                            {
                                CleanupCOMPort();
                                CannotReadCOMPortError();

                                continue;
                            }

                            if (readBuffer[0] != SFTT_SERIAL_FINAL_SIGNATURE)
                            {
                                CleanupCOMPort();
                                SerialFinalSignatureMismatchError(SFTT_SERIAL_FINAL_SIGNATURE, readBuffer[0]);

                                continue;
                            }

                            // TODO
                            MessageBoxW(NULL, L"Match", L"", MB_OK);

                            CleanupCOMPort();
                        }

                        continue;
                    }
                }
            }
        }
    }

    return 0;
}

BOOL StartReceiving(LPCWSTR portName)
{
    if (!OpenCOMPort(portName, &g_receivingCOMPortHandle))
    {
        return FALSE;
    }

    g_currentReceiveStage = RECEIVE_STAGE_WAITING_FOR_START_SIGNATURE;

    SetCommMask(g_receivingCOMPortHandle, EV_RXCHAR);

    g_receiverThreadHandle = CreateThread(NULL, 0, ReceiverThread, NULL, 0, NULL);

    g_isReceiving = TRUE;

    return TRUE;
}

DWORD WINAPI SenderThread(LPVOID lpParam)
{
    (void)lpParam;

    size_t totalSteps = 3 + SENDING_FILE_SIZE;
    ResetProgressBar();
    SetProgressBarRange(totalSteps);

    DWORD bytesWritten;

    uint64_t sendBuffer[3] = {
        SFTT_SERIAL_START_SIGNATURE,
        (uint64_t)SENDING_FILE_SIZE,
        (uint64_t)SENDING_FILE_NAME_SIZE};
    if (!WriteFile(SENDING_COM_PORT_HANDLE, sendBuffer, 3 * sizeof(uint64_t), &bytesWritten, NULL))
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

    if (!WriteFile(SENDING_COM_PORT_HANDLE, SENDING_FILE_NAME, SENDING_FILE_NAME_SIZE, &bytesWritten, NULL))
    {
        CannotWriteCOMPortError();

        goto CleanUp;
    }
    if (bytesWritten != SENDING_FILE_NAME_SIZE)
    {
        BytesWrittenMismatchError(SENDING_FILE_NAME_SIZE, bytesWritten);

        goto CleanUp;
    }

    StepProgressBar();

    BYTE binBuffer[4096];
    DWORD bytesWrittenTotal = 0;
    DWORD bytesRead;
    while (ReadFile(SENDING_FILE_HANDLE, binBuffer, sizeof(binBuffer), &bytesRead, NULL) && bytesRead > 0)
    {
        DWORD bytesWrittenThisTime;
        DWORD offset = 0;
        while (offset < bytesRead)
        {
            if (!WriteFile(
                    SENDING_COM_PORT_HANDLE,
                    binBuffer + offset,
                    bytesRead - offset,
                    &bytesWrittenThisTime,
                    NULL))
            {
                CannotWriteCOMPortError();

                goto CleanUp;
            }

            if (bytesWrittenThisTime == 0)
            {
                Sleep(1);

                continue;
            }

            AddStepToProgressBar(bytesWrittenThisTime);

            offset += bytesWrittenThisTime;
        }

        bytesWrittenTotal += offset;
    }

    if (bytesWrittenTotal != SENDING_FILE_SIZE)
    {
        BytesWrittenMismatchError(SENDING_FILE_SIZE, bytesWrittenTotal);

        goto CleanUp;
    }

    uint64_t sendBuffer2[1] = {SFTT_SERIAL_FINAL_SIGNATURE};
    if (!WriteFile(SENDING_COM_PORT_HANDLE, sendBuffer2, sizeof(uint64_t), &bytesWritten, NULL))
    {
        CannotWriteCOMPortError();

        goto CleanUp;
    }
    if (bytesWritten != sizeof(uint64_t))
    {
        BytesWrittenMismatchError(sizeof(sendBuffer2), bytesWritten);

        goto CleanUp;
    }

    StepProgressBar();

CleanUp:
    if (SENDING_COM_PORT_HANDLE != INVALID_HANDLE_VALUE)
    {
        CloseHandle(SENDING_COM_PORT_HANDLE);
        SENDING_COM_PORT_HANDLE = INVALID_HANDLE_VALUE;
    }
    if (SENDING_FILE_HANDLE != INVALID_HANDLE_VALUE)
    {
        CloseHandle(SENDING_FILE_HANDLE);
        SENDING_FILE_HANDLE = INVALID_HANDLE_VALUE;
    }
    if (SENDING_FILE_NAME != NULL)
    {
        free(SENDING_FILE_NAME);
        SENDING_FILE_NAME = NULL;
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

    SENDING_COM_PORT_HANDLE = hComPort;
    SENDING_FILE_HANDLE = handleFile;
    SENDING_FILE_NAME = _wcsdup(fileName);
    SENDING_FILE_NAME_SIZE = fileNameSize;
    SENDING_FILE_SIZE = fileSize.QuadPart;

    SENDER_THREAD_HANDLE = CreateThread(NULL, 0, SenderThread, NULL, 0, NULL);
}

void FinaliseSerial(void)
{
    if (SENDER_THREAD_HANDLE != INVALID_HANDLE_VALUE)
    {
        TerminateThread(SENDER_THREAD_HANDLE, 0);
    }

    if (SENDING_COM_PORT_HANDLE != INVALID_HANDLE_VALUE)
    {
        CloseHandle(SENDING_COM_PORT_HANDLE);
    }

    if (SENDING_FILE_HANDLE != INVALID_HANDLE_VALUE)
    {
        CloseHandle(SENDING_FILE_HANDLE);
    }

    if (SENDING_FILE_NAME != NULL)
    {
        free(SENDING_FILE_NAME);
    }

    if (g_isReceiving)
    {
        StopReceiving();
    }
}
