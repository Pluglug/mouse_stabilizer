/**
 * Mouse Stabilizer - Main Application Entry Point
 * 
 * Initializes Windows application components:
 * - Hidden window for message handling
 * - System tray icon and context menu
 * - Raw input registration for mouse capture
 * - Hotkey registration (Ctrl+Alt+S)
 * - Target pointer overlay window
 */

#include "mouse_stabilizer.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
#ifndef DEBUG
    // Hide console window in release builds
    HWND console_window = GetConsoleWindow();
    if (console_window != NULL) {
        ShowWindow(console_window, SW_HIDE);
    }
#endif
    const char* class_name = "MouseStabilizerWindow";
    WNDCLASS wc = {0};
    MSG msg;
    HHOOK mouse_hook = NULL;
    
    wc.lpfnWndProc = Hotkey_WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (!wc.hIcon) {
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
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
    
    StabilizerCore_Initialize(&g_stabilizer);
    Settings_Load();
    
    // Force DEBUG level for troubleshooting
    Settings_SetLogLevel(LOG_DEBUG);
    
    if (!SettingsUI_Initialize()) {
        LOG_WARN("Failed to initialize settings UI - continuing without settings window");
    }
    
    if (!RegisterHotKey(g_hidden_window, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'S')) {
        LOG_ERROR("Failed to register hotkey Ctrl+Alt+S");
    } else {
        LOG_INFO("Hotkey Ctrl+Alt+S registered successfully");
    }
    
    if (!TrayUI_CreateIcon(g_hidden_window)) {
        LOG_ERROR("Failed to create tray icon");
    } else {
        LOG_INFO("Tray icon created successfully");
    }
    
    if (!SetTimer(g_hidden_window, TIMER_ID, UPDATE_INTERVAL_MS, NULL)) {
        Settings_WriteLog("Failed to create update timer");
    } else {
        Settings_WriteLog("Update timer created successfully (interval: %dms)", UPDATE_INTERVAL_MS);
    }
    
    if (!SetTimer(g_hidden_window, DRAW_TIMER_ID, DRAW_INTERVAL_MS, NULL)) {
        Settings_WriteLog("Failed to create draw timer");
    } else {
        Settings_WriteLog("Draw timer created successfully (interval: %dms)", DRAW_INTERVAL_MS);
    }
    
    if (!TargetPointer_CreateWindow()) {
        LOG_WARN("Failed to create target window - continuing without target pointer");
    }
    
    if (!MouseInput_RegisterRawInput()) {
        LOG_WARN("Failed to register raw input - using fallback method");
    }
    
    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, MouseInput_LowLevelMouseProc, hInstance, 0);
    if (!mouse_hook) {
        LOG_ERROR("Failed to install mouse hook");
        MessageBox(NULL, "Failed to install mouse hook", "Error", MB_ICONERROR);
        return 1;
    } else {
        LOG_INFO("Mouse hook installed successfully");
    }
    
    Settings_WriteLog("Mouse Stabilizer started - Follow: %.2f, MinDist: %.1f, Ease: %d, Dual: %s, Delay: %dms, Enabled: %s",
             g_stabilizer.follow_strength, g_stabilizer.min_distance, g_stabilizer.ease_type,
             g_stabilizer.dual_mode ? "true" : "false", g_stabilizer.delay_start_ms, 
             g_stabilizer.enabled ? "true" : "false");
    
    TrayUI_UpdateIcon();
    
    while (g_running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (mouse_hook) {
        UnhookWindowsHookEx(mouse_hook);
        Settings_WriteLog("Mouse hook uninstalled");
    }
    
    KillTimer(g_hidden_window, TIMER_ID);
    KillTimer(g_hidden_window, DRAW_TIMER_ID);
    if (g_target_window) {
        DestroyWindow(g_target_window);
    }
    UnregisterHotKey(g_hidden_window, HOTKEY_ID);
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    Settings_Save();
    
    Settings_WriteLog("Mouse Stabilizer terminated");
    
    return 0;
}