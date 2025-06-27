#include "mouse_stabilizer.h"
#include <shellapi.h>

bool TrayUI_CreateIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = TRAY_ICON_ID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(g_nid.szTip, sizeof(g_nid.szTip), "Mouse Stabilizer - Enabled");
    
    return Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void TrayUI_UpdateIcon(void) {
    const char* status = g_stabilizer.enabled ? "Enabled" : "Disabled";
    const char* ease_name = "";
    
    switch (g_stabilizer.ease_type) {
        case EASE_LINEAR:
            ease_name = "Linear";
            break;
        case EASE_IN:
            ease_name = "Ease In";
            break;
        case EASE_OUT:
            ease_name = "Ease Out";
            break;
        case EASE_IN_OUT:
            ease_name = "Ease In-Out";
            break;
    }
    
    sprintf_s(g_nid.szTip, sizeof(g_nid.szTip), 
              "Mouse Stabilizer - %s\nEase: %s\nFollow: %.2f\nDelay: %lums",
              status, ease_name, g_stabilizer.follow_strength, (unsigned long)g_stabilizer.delay_start_ms);
    
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

void TrayUI_ShowContextMenu(HWND hwnd) {
    LOG_DEBUG("Creating context menu");
    HMENU hMenu = CreatePopupMenu();
    POINT pt;
    
    if (!hMenu) {
        LOG_ERROR("Failed to create popup menu: error code %lu", GetLastError());
        return;
    }
    
    GetCursorPos(&pt);
    LOG_DEBUG("Context menu position: (%ld, %ld)", pt.x, pt.y);
    
    // Simple tray menu with essential functions only
    const char* toggle_text = g_stabilizer.enabled ? "Disable Stabilizer" : "Enable Stabilizer";
    AppendMenu(hMenu, MF_STRING, 1001, toggle_text);
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    AppendMenu(hMenu, MF_STRING, 1002, "Settings...");
    
    char debug_text[256];
    sprintf_s(debug_text, sizeof(debug_text), "Debug Mode: %s", 
              Settings_GetLogLevelName(Settings_GetLogLevel()));
    AppendMenu(hMenu, MF_STRING, 1003, debug_text);
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 1004, "Exit");
    
    SetForegroundWindow(hwnd);
    LOG_DEBUG("Showing context menu");
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    LOG_DEBUG("Context menu closed");
    DestroyMenu(hMenu);
    LOG_DEBUG("Context menu destroyed");
}