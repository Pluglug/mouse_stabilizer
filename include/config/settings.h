#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdarg.h>

// Settings management functions
void Settings_Load(void);
void Settings_Save(void);

// Logging functions
void Settings_WriteLog(const char* format, ...);

#endif // SETTINGS_H