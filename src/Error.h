#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>
#include <windows.h>

void CannotOpenFileError(wchar_t *filePath);

void CannotGetFileSizeError(wchar_t *filePath);

void PleaseSpecifyPortError(void);

void PleaseSpecifyFilePathError(void);

void PleaseSpecifyDirectoryError(void);

void CannotOpenCOMPortError(wchar_t *portName);

void CannotReadCOMPortError(void);

void BytesReadMismatchError(DWORD expected, DWORD read);

void CannotWriteCOMPortError(void);

void BytesWrittenMismatchError(DWORD expected, DWORD written);

void SerialStartSignatureMismatchError(uint64_t expected, uint64_t read);

void SerialFinalSignatureMismatchError(uint64_t expected, uint64_t read);

#endif
