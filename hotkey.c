#include "mouse_stabilizer.h"

void Hotkey_ToggleStabilizer(void) {
    g_stabilizer.enabled = !g_stabilizer.enabled;
    
    Settings_WriteLog("Mouse stabilizer %s", g_stabilizer.enabled ? "enabled" : "disabled");
    Settings_Save();
    TrayUI_UpdateIcon();
}

LRESULT CALLBACK Hotkey_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) {
                Hotkey_ToggleStabilizer();
            }
            return 0;
            
        case WM_INPUT:
            MouseInput_ProcessRawInput(lParam);
            return 0;
            
        case WM_TIMER:
            if (wParam == TIMER_ID) {
                StabilizerCore_UpdatePosition(&g_stabilizer);
            } else if (wParam == DRAW_TIMER_ID) {
                TargetPointer_UpdateWindow();
            }
            return 0;
            
        case WM_TRAYICON:
            switch (lParam) {
                case WM_RBUTTONUP:
                    TrayUI_ShowContextMenu(hwnd);
                    break;
                case WM_LBUTTONDBLCLK:
                    Hotkey_ToggleStabilizer();
                    break;
            }
            return 0;
            
        case WM_COMMAND:
            LOG_DEBUG("WM_COMMAND received, wParam: %lu", (unsigned long)wParam);
            {
                int command_id = LOWORD(wParam);
                if (command_id >= MENU_PROFILE_BASE && command_id < MENU_PROFILE_BASE + MAX_PROFILES) {
                    int profile_index = command_id - MENU_PROFILE_BASE;
                    if (Settings_ApplyProfile(profile_index)) {
                        TargetPointer_UpdateSettings();
                        SettingsUI_UpdateControls();
                        TrayUI_UpdateIcon();
                    }
                    return 0;
                }
            }
            switch (LOWORD(wParam)) {
                case MENU_TOGGLE_STABILIZER:  // Toggle Stabilizer
                    LOG_DEBUG("Toggle Stabilizer command received");
                    Hotkey_ToggleStabilizer();
                    break;
                case MENU_SHOW_SETTINGS:  // Settings Window
                    LOG_DEBUG("Settings Window command received");
                    SettingsUI_ShowWindow();
                    LOG_DEBUG("SettingsUI_ShowWindow call completed");
                    break;
                case MENU_TOGGLE_DEBUG:  // Debug Mode Toggle
                    {
                        LogLevel current = Settings_GetLogLevel();
                        LogLevel next = (LogLevel)((current + 1) % (LOG_TRACE + 1));
                        Settings_SetLogLevel(next);
                        Settings_Save();
                    }
                    break;
                case MENU_EXIT_APP:  // Exit
                    g_running = false;
                    PostQuitMessage(0);
                    break;
            }
            return 0;
            
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
