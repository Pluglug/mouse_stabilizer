#include "mouse_stabilizer.h"

void Hotkey_ToggleStabilizer(void) {
    g_stabilizer.enabled = !g_stabilizer.enabled;
    
    Settings_WriteLog("Mouse stabilizer %s", g_stabilizer.enabled ? "enabled" : "disabled");
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
            switch (LOWORD(wParam)) {
                case 1001:
                    Hotkey_ToggleStabilizer();
                    break;
                case 1002:
                    if (g_stabilizer.ease_type == EASE_LINEAR) {
                        g_stabilizer.ease_type = EASE_IN;
                    } else if (g_stabilizer.ease_type == EASE_IN) {
                        g_stabilizer.ease_type = EASE_OUT;
                    } else if (g_stabilizer.ease_type == EASE_OUT) {
                        g_stabilizer.ease_type = EASE_IN_OUT;
                    } else {
                        g_stabilizer.ease_type = EASE_LINEAR;
                    }
                    Settings_WriteLog("Ease type changed to: %d", g_stabilizer.ease_type);
                    Settings_Save();
                    break;
                case 1003:
                    g_stabilizer.follow_strength += 0.05f;
                    if (g_stabilizer.follow_strength > 1.0f) {
                        g_stabilizer.follow_strength = 0.05f;
                    }
                    Settings_WriteLog("Follow strength: %.2f", g_stabilizer.follow_strength);
                    Settings_Save();
                    break;
                case 1004:
                    g_stabilizer.dual_mode = !g_stabilizer.dual_mode;
                    Settings_WriteLog("Dual mode: %s", g_stabilizer.dual_mode ? "enabled" : "disabled");
                    Settings_Save();
                    break;
                case 1005:
                    g_stabilizer.delay_start_ms += 50;
                    if (g_stabilizer.delay_start_ms > 500) {
                        g_stabilizer.delay_start_ms = 0;
                    }
                    Settings_WriteLog("Delay start: %dms", g_stabilizer.delay_start_ms);
                    Settings_Save();
                    break;
                case 1006:
                    g_stabilizer.target_show_distance += 2.0f;
                    if (g_stabilizer.target_show_distance > 20.0f) {
                        g_stabilizer.target_show_distance = 2.0f;
                    }
                    Settings_WriteLog("Target show distance: %.1f", g_stabilizer.target_show_distance);
                    Settings_Save();
                    break;
                case 1008:
                    // Cycle through log levels: ERROR -> WARN -> INFO -> DEBUG -> TRACE -> ERROR
                    {
                        LogLevel current = Settings_GetLogLevel();
                        LogLevel next = (LogLevel)((current + 1) % (LOG_TRACE + 1));
                        Settings_SetLogLevel(next);
                        Settings_Save();
                    }
                    break;
                case 1007:
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