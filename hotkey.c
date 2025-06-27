#include "mouse_stabilizer.h"

void ToggleStabilizer(void) {
    g_stabilizer.enabled = !g_stabilizer.enabled;
    
    WriteLog("Mouse stabilizer %s", g_stabilizer.enabled ? "enabled" : "disabled");
    UpdateTrayIcon();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) {
                ToggleStabilizer();
            }
            return 0;
            
        case WM_INPUT:
            ProcessRawInput(lParam);
            return 0;
            
        case WM_TIMER:
            if (wParam == TIMER_ID) {
                UpdateSmoothPosition(&g_stabilizer);
            } else if (wParam == DRAW_TIMER_ID) {
                UpdateTargetWindow();
            }
            return 0;
            
        case WM_TRAYICON:
            switch (lParam) {
                case WM_RBUTTONUP:
                    ShowContextMenu(hwnd);
                    break;
                case WM_LBUTTONDBLCLK:
                    ToggleStabilizer();
                    break;
            }
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1001:
                    ToggleStabilizer();
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
                    WriteLog("Ease type changed to: %d", g_stabilizer.ease_type);
                    SaveSettings();
                    break;
                case 1003:
                    g_stabilizer.follow_strength += 0.05f;
                    if (g_stabilizer.follow_strength > 1.0f) {
                        g_stabilizer.follow_strength = 0.05f;
                    }
                    WriteLog("Follow strength: %.2f", g_stabilizer.follow_strength);
                    SaveSettings();
                    break;
                case 1004:
                    g_stabilizer.dual_mode = !g_stabilizer.dual_mode;
                    WriteLog("Dual mode: %s", g_stabilizer.dual_mode ? "enabled" : "disabled");
                    SaveSettings();
                    break;
                case 1005:
                    g_stabilizer.delay_start_ms += 50;
                    if (g_stabilizer.delay_start_ms > 500) {
                        g_stabilizer.delay_start_ms = 0;
                    }
                    WriteLog("Delay start: %dms", g_stabilizer.delay_start_ms);
                    SaveSettings();
                    break;
                case 1006:
                    g_stabilizer.target_show_distance += 2.0f;
                    if (g_stabilizer.target_show_distance > 20.0f) {
                        g_stabilizer.target_show_distance = 2.0f;
                    }
                    WriteLog("Target show distance: %.1f", g_stabilizer.target_show_distance);
                    SaveSettings();
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