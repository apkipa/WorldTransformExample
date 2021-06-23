#include "public.h"

void PrintErrorA(const char *fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    printf("\e[1;31mError\e[1;37m: ");
    vprintf(fmt, vl);
    printf("\e[0m\n");
    va_end(vl);
}
void PrintErrorW(const wchar_t *fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    wprintf(L"\e[1;31mError\e[1;37m: ");
    vwprintf(fmt, vl);
    wprintf(L"\e[0m\n");
    va_end(vl);
}

bool wcstof_strict(const wchar_t *str, float *pVal) {
    wchar_t *ptr;
    float fRet;
    fRet = wcstof(str, &ptr);
    if (*ptr != L'\0') {
        return false;
    }
    *pVal = fRet;
    return true;
}
