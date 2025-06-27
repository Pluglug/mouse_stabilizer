#ifndef TRAY_UI_H
#define TRAY_UI_H

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
#endif

// Tray UI constants
#define HOTKEY_ID 1
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)
#define WM_UPDATE_TIMER (WM_USER + 2)
#define WM_DRAW_TIMER (WM_USER + 3)
#define TIMER_ID 1
#define DRAW_TIMER_ID 2

// Global tray icon data
extern NOTIFYICONDATA g_nid;

// Tray UI functions
bool TrayUI_CreateIcon(HWND hwnd);
void TrayUI_UpdateIcon(void);
void TrayUI_ShowContextMenu(HWND hwnd);

#endif // TRAY_UI_H