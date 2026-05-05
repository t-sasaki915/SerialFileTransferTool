#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#define MAX_PATH_LENGTH 260

wchar_t *GetFileName(wchar_t *filePath);

int Format(wchar_t *buffer, size_t count, const wchar_t *format, ...);

#endif
