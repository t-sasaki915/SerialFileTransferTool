#ifndef ERROR_H
#define ERROR_H

#include <windows.h>

void CannotOpenCOMPortError(wchar_t *portName);

void PleaseSpecifyPortError(void);

void PleaseSpecifyFilePathError(void);

#endif
