/**
 * Mouse Input Processing - Raw Input API Integration
 * 
 * Handles Windows Raw Input registration and processing to capture
 * high-precision mouse movement deltas before Windows applies acceleration.
 */

#include "mouse_stabilizer.h"

static bool g_in_stabilizer_update = false;  // Prevent infinite recursion
static RAWINPUTDEVICE g_rid[1];              // Raw input device registration

bool MouseInput_RegisterRawInput(void) {
    g_rid[0].usUsagePage = 0x01;
    g_rid[0].usUsage = 0x02;
    g_rid[0].dwFlags = RIDEV_INPUTSINK;
    g_rid[0].hwndTarget = g_hidden_window;
    
    if (!RegisterRawInputDevices(g_rid, 1, sizeof(g_rid[0]))) {
        Settings_WriteLog("Failed to register raw input device");
        return false;
    }
    
    Settings_WriteLog("Raw input device registered successfully");
    return true;
}

void MouseInput_ProcessRawInput(LPARAM lParam) {
    if (g_in_stabilizer_update) return;
    
    UINT dwSize = 0;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    
    if (dwSize == 0) return;
    
    BYTE* lpb = malloc(dwSize);
    if (!lpb) return;
    
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
        free(lpb);
        return;
    }
    
    RAWINPUT* raw = (RAWINPUT*)lpb;
    
    if (raw->header.dwType == RIM_TYPEMOUSE && g_stabilizer.enabled) {
        if (raw->data.mouse.lLastX != 0 || raw->data.mouse.lLastY != 0) {
            g_in_stabilizer_update = true;
            StabilizerCore_AddMouseDelta(&g_stabilizer, (float)raw->data.mouse.lLastX, (float)raw->data.mouse.lLastY);
            g_in_stabilizer_update = false;
        }
    }
    
    free(lpb);
}

LRESULT CALLBACK MouseInput_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && g_stabilizer.enabled && !g_in_stabilizer_update) {
        if (wParam == WM_MOUSEMOVE) {
            return 1;
        }
    }
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}