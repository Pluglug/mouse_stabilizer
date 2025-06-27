#include "mouse_stabilizer.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    const char* class_name = "MouseStabilizerWindow";
    WNDCLASS wc = {0};
    MSG msg;
    HHOOK mouse_hook = NULL;
    
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_ICONERROR);
        return 1;
    }
    
    g_hidden_window = CreateWindow(
        class_name, "Mouse Stabilizer",
        0, 0, 0, 0, 0,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hidden_window) {
        MessageBox(NULL, "Failed to create window", "Error", MB_ICONERROR);
        return 1;
    }
    
    InitializeStabilizer(&g_stabilizer);
    LoadSettings();
    
    InitializeKalman(&g_kalman_x, 0.01f, 0.1f);
    InitializeKalman(&g_kalman_y, 0.01f, 0.1f);
    
    if (!RegisterHotKey(g_hidden_window, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'S')) {
        WriteLog("Failed to register hotkey Ctrl+Alt+S");
    } else {
        WriteLog("Hotkey Ctrl+Alt+S registered successfully");
    }
    
    if (!CreateTrayIcon(g_hidden_window)) {
        WriteLog("Failed to create tray icon");
    } else {
        WriteLog("Tray icon created successfully");
    }
    
    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
    if (!mouse_hook) {
        WriteLog("Failed to install mouse hook");
        MessageBox(NULL, "Failed to install mouse hook", "Error", MB_ICONERROR);
        return 1;
    } else {
        WriteLog("Mouse hook installed successfully");
    }
    
    WriteLog("Mouse Stabilizer started - Filter: %d, Smoothing: %.2f, Threshold: %.1f, Enabled: %s",
             g_stabilizer.filter_type, g_stabilizer.smoothing_strength, g_stabilizer.threshold,
             g_stabilizer.enabled ? "true" : "false");
    
    UpdateTrayIcon();
    
    while (g_running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (mouse_hook) {
        UnhookWindowsHookEx(mouse_hook);
        WriteLog("Mouse hook uninstalled");
    }
    
    UnregisterHotKey(g_hidden_window, HOTKEY_ID);
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    SaveSettings();
    
    WriteLog("Mouse Stabilizer terminated");
    
    return 0;
}