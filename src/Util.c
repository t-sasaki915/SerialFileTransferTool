#include <windows.h>

typedef int (*PVSWPRINTF_S)(wchar_t *, size_t, const wchar_t *, va_list);

PVSWPRINTF_S FORMATTER = NULL;

wchar_t *GetFileName(wchar_t *filePath)
{
    wchar_t *lastSlash = wcsrchr(filePath, L'\\');
    if (!lastSlash)
    {
        return filePath;
    }

    return lastSlash + 1;
}

int Format(wchar_t *buffer, size_t count, const wchar_t *format, ...)
{
    int result = -1;

    va_list args;
    va_start(args, format);

    if (FORMATTER == NULL)
    {
        union {
            FARPROC addr;
            PVSWPRINTF_S func;
        } converter;

        HMODULE msvcrt = GetModuleHandleW(L"msvcrt.dll");
        converter.addr = GetProcAddress(msvcrt, "vswprintf_s");

        if (converter.addr != NULL)
        {
            FORMATTER = converter.func;
        }
        else
        {
            converter.addr = GetProcAddress(msvcrt, "_vsnwprintf");
            FORMATTER = converter.func;
        }
    }

    result = FORMATTER(buffer, count, format, args);

    va_end(args);

    return result;
}
