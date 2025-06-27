#include "mouse_stabilizer.h"
#include <shellapi.h>

bool CreateTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = TRAY_ICON_ID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(g_nid.szTip, sizeof(g_nid.szTip), "Mouse Stabilizer - Enabled");
    
    return Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void UpdateTrayIcon(void) {
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
              "Mouse Stabilizer - %s\nEase: %s\nFollow: %.2f\nDelay: %dms",
              status, ease_name, g_stabilizer.follow_strength, g_stabilizer.delay_start_ms);
    
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

void ShowContextMenu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();
    POINT pt;
    
    GetCursorPos(&pt);
    
    const char* toggle_text = g_stabilizer.enabled ? "Disable Stabilizer" : "Enable Stabilizer";
    AppendMenu(hMenu, MF_STRING, 1001, toggle_text);
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    const char* ease_names[] = {"Linear", "Ease In", "Ease Out", "Ease In-Out"};
    char ease_text[256];
    sprintf_s(ease_text, sizeof(ease_text), "Ease: %s (Click to change)", 
              ease_names[g_stabilizer.ease_type]);
    AppendMenu(hMenu, MF_STRING, 1002, ease_text);
    
    char follow_text[256];
    sprintf_s(follow_text, sizeof(follow_text), "Follow: %.2f (Click to change)", 
              g_stabilizer.follow_strength);
    AppendMenu(hMenu, MF_STRING, 1003, follow_text);
    
    char dual_text[256];
    sprintf_s(dual_text, sizeof(dual_text), "Dual Mode: %s (Click to toggle)", 
              g_stabilizer.dual_mode ? "On" : "Off");
    AppendMenu(hMenu, MF_STRING, 1004, dual_text);
    
    char delay_text[256];
    sprintf_s(delay_text, sizeof(delay_text), "Delay: %dms (Click to change)", 
              g_stabilizer.delay_start_ms);
    AppendMenu(hMenu, MF_STRING, 1005, delay_text);
    
    char distance_text[256];
    sprintf_s(distance_text, sizeof(distance_text), "Target Distance: %.1f (Click to change)", 
              g_stabilizer.target_show_distance);
    AppendMenu(hMenu, MF_STRING, 1006, distance_text);
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 1007, "Exit");
    
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}