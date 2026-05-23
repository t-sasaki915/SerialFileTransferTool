#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>
#include <windows.h>

#include "Resource.h"

typedef enum
{
    ERROR_TYPE_CANNOT_OPEN_FILE = IDSTRING_CANNOT_OPEN_FILE_ERROR,
    ERROR_TYPE_CANNOT_GET_FILE_SIZE = IDSTRING_CANNOT_GET_FILE_SIZE_ERROR,
    ERROR_TYPE_CANNOT_WRITE_FILE = IDSTRING_CANNOT_WRITE_FILE_ERROR,
    ERROR_TYPE_PORT_NOT_SPECIFIED = IDSTRING_PORT_NOT_SPECIFIED_ERROR,
    ERROR_TYPE_FILE_PATH_NOT_SPECIFIED = IDSTRING_FILE_PATH_NOT_SPECIFIED_ERROR,
    ERROR_TYPE_DESTINATION_NOT_SPECIFIED = IDSTRING_DESTINATION_NOT_SPECIFIED_ERROR,
    ERROR_TYPE_CANNOT_OPEN_COM_PORT = IDSTRING_CANNOT_OPEN_COM_PORT_ERROR,
    ERROR_TYPE_CANNOT_READ_COM_PORT = IDSTRING_CANNOT_READ_COM_PORT_ERROR,
    ERROR_TYPE_CANNOT_WRITE_COM_PORT = IDSTRING_CANNOT_WRITE_COM_PORT_ERROR,
    ERROR_TYPE_BYTES_READ_MISMATCH = IDSTRING_BYTES_READ_MISMATCH_ERROR,
    ERROR_TYPE_BYTES_WRITTEN_MISMATCH = IDSTRING_BYTES_WRITTEN_MISMATCH_ERROR,
    ERROR_TYPE_SHA1_INPUT = IDSTRING_SHA1_INPUT_ERROR,
    ERROR_TYPE_SHA1_CALCULATION = IDSTRING_SHA1_CALCULATION_ERROR,
    ERROR_TYPE_SHA1_MISMATCH = IDSTRING_SHA1_MISMATCH_ERROR,
    ERROR_TYPE_SERIAL_START_SIGNATURE_MISMATCH = IDSTRING_SERIAL_START_SIGNATURE_MISMATCH_ERROR,
    ERROR_TYPE_SERIAL_FINAL_SIGNATURE_MISMATCH = IDSTRING_SERIAL_FINAL_SIGNATURE_MISMATCH_ERROR
} ErrorType;

typedef struct
{
    union {
        uint64_t u64;
        DWORD dword;
        wchar_t *wchar;
    } expected;

    union {
        uint64_t u64;
        DWORD dword;
        wchar_t *wchar;
    } actual;
} MismatchErrorDetails;

typedef struct
{
    ErrorType errorType;
    DWORD lastErrorCode;
    union {
        wchar_t *filePath;
        wchar_t *portName;
        MismatchErrorDetails mismatchError;
    } errorDetails;
} ErrorContext;

void CannotOpenFileError(wchar_t *filePath);

void CannotGetFileSizeError(wchar_t *filePath);

void PleaseSpecifyPortError(void);

void PleaseSpecifyFilePathError(void);

void PleaseSpecifyDirectoryError(void);

void CannotOpenCOMPortError(wchar_t *portName);

void CannotReadCOMPortError(void);

void BytesReadMismatchError(DWORD expected, DWORD read);

void CannotWriteCOMPortError(void);

void CannotWriteFileError(void);

void BytesWrittenMismatchError(DWORD expected, DWORD written);

void SHA1InputError(void);

void SHA1CalculationError(void);

void SHA1MismatchError(uint8_t *expected, uint8_t *actual);

void SerialStartSignatureMismatchError(uint64_t expected, uint64_t read);

void SerialFinalSignatureMismatchError(uint64_t expected, uint64_t read);

#endif
