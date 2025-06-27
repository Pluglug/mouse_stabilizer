#ifndef TRAY_UI_H
#define TRAY_UI_H

#include <windows.h>
#include <stdbool.h>

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