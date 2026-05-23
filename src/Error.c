#include <stdint.h>
#include <windows.h>

#include "Error.h"
#include "SHA1.h"
#include "UI.h"

void CannotOpenFileError(wchar_t *filePath)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_CANNOT_OPEN_FILE;
    ctx->lastErrorCode = GetLastError();
    ctx->errorDetails.filePath = _wcsdup(filePath);

    RequestErrorDialog(ctx);
}

void CannotGetFileSizeError(wchar_t *filePath)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_CANNOT_GET_FILE_SIZE;
    ctx->lastErrorCode = GetLastError();
    ctx->errorDetails.filePath = _wcsdup(filePath);

    RequestErrorDialog(ctx);
}

void PleaseSpecifyPortError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_PORT_NOT_SPECIFIED;

    RequestErrorDialog(ctx);
}

void PleaseSpecifyFilePathError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_FILE_PATH_NOT_SPECIFIED;

    RequestErrorDialog(ctx);
}

void PleaseSpecifyDirectoryError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_DESTINATION_NOT_SPECIFIED;

    RequestErrorDialog(ctx);
}

void CannotOpenCOMPortError(wchar_t *portName)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_CANNOT_OPEN_COM_PORT;
    ctx->lastErrorCode = GetLastError();
    ctx->errorDetails.portName = _wcsdup(portName);

    RequestErrorDialog(ctx);
}

void CannotReadCOMPortError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_CANNOT_READ_COM_PORT;
    ctx->lastErrorCode = GetLastError();

    RequestErrorDialog(ctx);
}

void BytesReadMismatchError(DWORD expected, DWORD read)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_BYTES_READ_MISMATCH;
    ctx->errorDetails.mismatchError.expected.dword = expected;
    ctx->errorDetails.mismatchError.actual.dword = read;

    RequestErrorDialog(ctx);
}

void CannotWriteCOMPortError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_CANNOT_WRITE_COM_PORT;
    ctx->lastErrorCode = GetLastError();

    RequestErrorDialog(ctx);
}

void CannotWriteFileError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_CANNOT_WRITE_FILE;
    ctx->lastErrorCode = GetLastError();

    RequestErrorDialog(ctx);
}

void BytesWrittenMismatchError(DWORD expected, DWORD written)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_BYTES_WRITTEN_MISMATCH;
    ctx->errorDetails.mismatchError.expected.dword = expected;
    ctx->errorDetails.mismatchError.actual.dword = written;

    RequestErrorDialog(ctx);
}

void SHA1InputError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_SHA1_INPUT;

    RequestErrorDialog(ctx);
}

void SHA1CalculationError(void)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_SHA1_CALCULATION;

    RequestErrorDialog(ctx);
}

void SHA1MismatchError(uint8_t *expected, uint8_t *actual)
{
    wchar_t decodedExpected[SHA1_HASH_TEXT_SIZE];
    DecodeSHA1Hash(expected, decodedExpected);
    wchar_t decodedActual[SHA1_HASH_TEXT_SIZE];
    DecodeSHA1Hash(actual, decodedActual);

    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_SHA1_MISMATCH;
    ctx->errorDetails.mismatchError.expected.wchar = _wcsdup(decodedExpected);
    ctx->errorDetails.mismatchError.actual.wchar = _wcsdup(decodedActual);

    RequestErrorDialog(ctx);
}

void SerialStartSignatureMismatchError(uint64_t expected, uint64_t read)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_SERIAL_START_SIGNATURE_MISMATCH;
    ctx->errorDetails.mismatchError.expected.u64 = expected;
    ctx->errorDetails.mismatchError.actual.u64 = read;

    RequestErrorDialog(ctx);
}

void SerialFinalSignatureMismatchError(uint64_t expected, uint64_t read)
{
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errorType = ERROR_TYPE_SERIAL_FINAL_SIGNATURE_MISMATCH;
    ctx->errorDetails.mismatchError.expected.u64 = expected;
    ctx->errorDetails.mismatchError.actual.u64 = read;

    RequestErrorDialog(ctx);
}
