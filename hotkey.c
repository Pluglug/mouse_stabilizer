#include "mouse_stabilizer.h"

void ToggleStabilizer(void) {
    g_stabilizer.enabled = !g_stabilizer.enabled;
    
    WriteLog("Mouse stabilizer %s", g_stabilizer.enabled ? "enabled" : "disabled");
    UpdateTrayIcon();
    
    if (g_stabilizer.enabled) {
        InitializeKalman(&g_kalman_x, 0.01f, 0.1f);
        InitializeKalman(&g_kalman_y, 0.01f, 0.1f);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) {
                ToggleStabilizer();
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
                    if (g_stabilizer.filter_type == FILTER_MOVING_AVERAGE) {
                        g_stabilizer.filter_type = FILTER_EXPONENTIAL;
                    } else if (g_stabilizer.filter_type == FILTER_EXPONENTIAL) {
                        g_stabilizer.filter_type = FILTER_KALMAN;
                    } else {
                        g_stabilizer.filter_type = FILTER_MOVING_AVERAGE;
                    }
                    WriteLog("Filter changed to: %d", g_stabilizer.filter_type);
                    SaveSettings();
                    break;
                case 1003:
                    g_stabilizer.smoothing_strength += 0.1f;
                    if (g_stabilizer.smoothing_strength > 1.0f) {
                        g_stabilizer.smoothing_strength = 0.1f;
                    }
                    WriteLog("Smoothing strength: %.1f", g_stabilizer.smoothing_strength);
                    SaveSettings();
                    break;
                case 1004:
                    g_stabilizer.threshold += 1.0f;
                    if (g_stabilizer.threshold > 20.0f) {
                        g_stabilizer.threshold = 1.0f;
                    }
                    WriteLog("Threshold: %.1f", g_stabilizer.threshold);
                    SaveSettings();
                    break;
                case 1005:
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