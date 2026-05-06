#include <stdint.h>
#include <windows.h>

#include "UI.h"
#include "Util.h"

#define CANNOT_OPEN_FILE_ERROR_MSG L"Cannot open the file '%ls': %lu."
#define CANNOT_OPEN_FILE_ERROR_MSG_LENGTH 350

#define CANNOT_GET_FILE_SIZE_ERROR_MSG L"Cannot get the size of the file '%ls': %lu."
#define CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH 400

#define PLEASE_SPECIFY_PORT_ERROR_MSG L"Please specify a port."

#define PLEASE_SPECIFY_FILE_PATH_ERROR_MSG L"Please specify a filepath."

#define PLEASE_SPECIFY_DIRECTORY_ERROR_MSG L"Please specify a directory."

#define CANNOT_OPEN_COM_PORT_ERROR_MSG L"Cannot open the COM port '%ls': %lu."
#define CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH 100

#define CANNOT_READ_COM_PORT_ERROR_MSG L"Cannot read COM port: '%lu'."
#define CANNOT_READ_COM_PORT_ERROR_MSG_LENGTH 30

#define BYTES_READ_MISMATCH_ERROR_MSG L"Bytes read mismatch. Expected: %lu, Read: %lu."
#define BYTES_READ_MISMATCH_ERROR_MSG_LENGTH 100

#define CANNOT_WRITE_COM_PORT_ERROR_MSG L"Cannot write to COM port: '%lu'."
#define CANNOT_WRITE_COM_PORT_ERROR_MSG_LENGTH 50

#define BYTES_WRITTEN_MISMATCH_ERROR_MSG L"Bytes written mismatch. Expected: %lu, Written: %lu."
#define BYTES_WRITTEN_MISMATCH_ERROR_MSG_LENGTH 100

#define SERIAL_START_SIGNATURE_MISMATCH_ERROR_MSG                                                                      \
    L"Serial start signature mismatch. Expected: 0x%016I64X, Read: 0x%016I64X."
#define SERIAL_START_SIGNATURE_MISMATCH_ERROR_MSG_LENGTH 150

#define SERIAL_FINAL_SIGNATURE_MISMATCH_ERROR_MSG                                                                      \
    L"Serial final signature mismatch. Expected: 0x%016I64X, Read: 0x%016I64X."
#define SERIAL_FINAL_SIGNATURE_MISMATCH_ERROR_MSG_LENGTH 150

void CannotOpenFileError(wchar_t *filePath)
{
    wchar_t msg[CANNOT_OPEN_FILE_ERROR_MSG_LENGTH];
    Format(msg, CANNOT_OPEN_FILE_ERROR_MSG_LENGTH, CANNOT_OPEN_FILE_ERROR_MSG, filePath, GetLastError());

    RequestErrorDialog(msg);
}

void CannotGetFileSizeError(wchar_t *filePath)
{
    wchar_t msg[CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH];
    Format(msg, CANNOT_GET_FILE_SIZE_ERROR_MSG_LENGTH, CANNOT_GET_FILE_SIZE_ERROR_MSG, filePath, GetLastError());

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

void PleaseSpecifyDirectoryError(void)
{
    RequestErrorDialog(PLEASE_SPECIFY_DIRECTORY_ERROR_MSG);
}

void CannotOpenCOMPortError(wchar_t *portName)
{
    wchar_t msg[CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH];
    Format(msg, CANNOT_OPEN_COM_PORT_ERROR_MSG_LENGTH, CANNOT_OPEN_COM_PORT_ERROR_MSG, portName, GetLastError());

    RequestErrorDialog(msg);
}

void CannotReadCOMPortError(void)
{
    wchar_t msg[CANNOT_READ_COM_PORT_ERROR_MSG_LENGTH];
    Format(msg, CANNOT_READ_COM_PORT_ERROR_MSG_LENGTH, CANNOT_READ_COM_PORT_ERROR_MSG, GetLastError());

    RequestErrorDialog(msg);
}

void BytesReadMismatchError(DWORD expected, DWORD read)
{
    wchar_t msg[BYTES_READ_MISMATCH_ERROR_MSG_LENGTH];
    Format(msg, BYTES_READ_MISMATCH_ERROR_MSG_LENGTH, BYTES_READ_MISMATCH_ERROR_MSG, expected, read);

    RequestErrorDialog(msg);
}

void CannotWriteCOMPortError(void)
{
    wchar_t msg[CANNOT_WRITE_COM_PORT_ERROR_MSG_LENGTH];
    Format(msg, CANNOT_WRITE_COM_PORT_ERROR_MSG_LENGTH, CANNOT_WRITE_COM_PORT_ERROR_MSG, GetLastError());

    RequestErrorDialog(msg);
}

void BytesWrittenMismatchError(DWORD expected, DWORD written)
{
    wchar_t msg[BYTES_WRITTEN_MISMATCH_ERROR_MSG_LENGTH];
    Format(msg, BYTES_WRITTEN_MISMATCH_ERROR_MSG_LENGTH, BYTES_WRITTEN_MISMATCH_ERROR_MSG, expected, written);

    RequestErrorDialog(msg);
}

void SerialStartSignatureMismatchError(uint64_t expected, uint64_t read)
{
    wchar_t msg[SERIAL_START_SIGNATURE_MISMATCH_ERROR_MSG_LENGTH];
    Format(
        msg,
        SERIAL_START_SIGNATURE_MISMATCH_ERROR_MSG_LENGTH,
        SERIAL_START_SIGNATURE_MISMATCH_ERROR_MSG,
        expected,
        read);

    RequestErrorDialog(msg);
}

void SerialFinalSignatureMismatchError(uint64_t expected, uint64_t read)
{
    wchar_t msg[SERIAL_FINAL_SIGNATURE_MISMATCH_ERROR_MSG_LENGTH];
    Format(
        msg,
        SERIAL_FINAL_SIGNATURE_MISMATCH_ERROR_MSG_LENGTH,
        SERIAL_FINAL_SIGNATURE_MISMATCH_ERROR_MSG,
        expected,
        read);

    RequestErrorDialog(msg);
}
