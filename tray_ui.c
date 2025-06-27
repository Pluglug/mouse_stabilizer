#include "mouse_stabilizer.h"
#include <shellapi.h>

NOTIFYICONDATA g_nid = {0};

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
    const char* filter_name = "";
    
    switch (g_stabilizer.filter_type) {
        case FILTER_MOVING_AVERAGE:
            filter_name = "Moving Average";
            break;
        case FILTER_EXPONENTIAL:
            filter_name = "Exponential";
            break;
        case FILTER_KALMAN:
            filter_name = "Kalman";
            break;
    }
    
    sprintf_s(g_nid.szTip, sizeof(g_nid.szTip), 
              "Mouse Stabilizer - %s\nFilter: %s\nSmoothing: %.1f\nThreshold: %.1f",
              status, filter_name, g_stabilizer.smoothing_strength, g_stabilizer.threshold);
    
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

void ShowContextMenu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();
    POINT pt;
    
    GetCursorPos(&pt);
    
    const char* toggle_text = g_stabilizer.enabled ? "Disable Stabilizer" : "Enable Stabilizer";
    AppendMenu(hMenu, MF_STRING, 1001, toggle_text);
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    const char* filter_names[] = {"Moving Average", "Exponential", "Kalman"};
    char filter_text[256];
    sprintf_s(filter_text, sizeof(filter_text), "Filter: %s (Click to change)", 
              filter_names[g_stabilizer.filter_type]);
    AppendMenu(hMenu, MF_STRING, 1002, filter_text);
    
    char smoothing_text[256];
    sprintf_s(smoothing_text, sizeof(smoothing_text), "Smoothing: %.1f (Click to change)", 
              g_stabilizer.smoothing_strength);
    AppendMenu(hMenu, MF_STRING, 1003, smoothing_text);
    
    char threshold_text[256];
    sprintf_s(threshold_text, sizeof(threshold_text), "Threshold: %.1f (Click to change)", 
              g_stabilizer.threshold);
    AppendMenu(hMenu, MF_STRING, 1004, threshold_text);
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 1005, "Exit");
    
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}