#include <windows.h>

typedef int (*PVSWPRINTF_S)(wchar_t *, size_t, const wchar_t *, va_list);

PVSWPRINTF_S g_formatterFunc = NULL;

void InitialiseUtilFunctions(void)
{
    union {
        FARPROC addr;
        PVSWPRINTF_S func;
    } converter;

    HMODULE msvcrt = GetModuleHandleW(L"msvcrt.dll");
    converter.addr = GetProcAddress(msvcrt, "vswprintf_s");

    if (converter.addr != NULL)
    {
        g_formatterFunc = converter.func;
    }
    else
    {
        converter.addr = GetProcAddress(msvcrt, "_vsnwprintf");
        g_formatterFunc = converter.func;
    }
}

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

    result = g_formatterFunc(buffer, count, format, args);

    va_end(args);

    return result;
}
