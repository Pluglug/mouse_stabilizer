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
    #define strcpy_s(dest, size, src) do { \
        strncpy(dest, src, (size)-1); \
        *((dest) + (size) - 1) = '\0'; \
    } while(0)
    // For MinGW/non-MSVC compilers on Windows, use standard functions
    #define localtime_s(timeptr, timer) do { *(timeptr) = *localtime(timer); } while(0)
    #define fopen_s(pFile, filename, mode) ((*(pFile) = fopen(filename, mode)) ? 0 : 1)
#endif

// Log levels for debugging and output control
typedef enum {
    LOG_ERROR,      // Critical errors only
    LOG_WARN,       // Warnings and errors
    LOG_INFO,       // General information (default)
    LOG_DEBUG,      // Detailed debugging information
    LOG_TRACE       // Very detailed trace information
} LogLevel;

// Global log level setting
extern LogLevel g_log_level;

// Settings management functions
void Settings_Load(void);
void Settings_Save(void);

// Enhanced logging functions
void Settings_WriteLog(const char* format, ...);
void Settings_WriteLogLevel(LogLevel level, const char* format, ...);
void Settings_SetLogLevel(LogLevel level);
LogLevel Settings_GetLogLevel(void);
const char* Settings_GetLogLevelName(LogLevel level);

// Convenient logging macros
#define LOG_ERROR(...) Settings_WriteLogLevel(LOG_ERROR, __VA_ARGS__)
#define LOG_WARN(...) Settings_WriteLogLevel(LOG_WARN, __VA_ARGS__)
#define LOG_INFO(...) Settings_WriteLogLevel(LOG_INFO, __VA_ARGS__)
#define LOG_DEBUG(...) Settings_WriteLogLevel(LOG_DEBUG, __VA_ARGS__)
#define LOG_TRACE(...) Settings_WriteLogLevel(LOG_TRACE, __VA_ARGS__)

#endif // SETTINGS_H