#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#define S_FREE(ptr)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (ptr != NULL)                                                                                               \
        {                                                                                                              \
            free(ptr);                                                                                                 \
            ptr = NULL;                                                                                                \
        }                                                                                                              \
    } while (0)

#define S_CLOSEHANDLE(hndl)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (hndl != INVALID_HANDLE_VALUE)                                                                              \
        {                                                                                                              \
            CloseHandle(hndl);                                                                                         \
            hndl = INVALID_HANDLE_VALUE;                                                                               \
        }                                                                                                              \
    } while (0)

void InitialiseFormatFunction(void);

wchar_t *GetFileName(wchar_t *filePath);

int Format(wchar_t *buffer, size_t count, const wchar_t *format, ...);

#endif
