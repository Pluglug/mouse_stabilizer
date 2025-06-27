#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Enable Microsoft secure functions
#ifdef _MSC_VER
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS
    #endif
#else
    // For non-MSVC compilers, provide sprintf_s compatibility
    #define sprintf_s snprintf
    #define strcpy_s(dest, size, src) strncpy(dest, src, size-1); dest[size-1] = '\0'
    // For MinGW/non-MSVC compilers on Windows, use standard functions
    #define localtime_s(timeptr, timer) (*(timeptr) = *localtime(timer), 0)
    #define fopen_s(pFile, filename, mode) ((*(pFile) = fopen(filename, mode)) ? 0 : 1)
#endif

// Settings management functions
void Settings_Load(void);
void Settings_Save(void);

// Logging functions
void Settings_WriteLog(const char* format, ...);

#endif // SETTINGS_H