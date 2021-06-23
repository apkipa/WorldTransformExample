#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define _USE_MATH_DEFINES

#include <sdkddkver.h>

#include <Windows.h>
#include <stdbool.h>
#include <dwmapi.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void PrintErrorA(const char *fmt, ...);
void PrintErrorW(const wchar_t *fmt, ...);

bool wcstof_strict(const wchar_t *str, float *pVal);
