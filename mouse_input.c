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
    if (!g_hidden_window) {
        LOG_ERROR("Cannot register raw input: hidden window not initialized");
        return false;
    }
    
    g_rid[0].usUsagePage = 0x01;    // HID_USAGE_PAGE_GENERIC
    g_rid[0].usUsage = 0x02;        // HID_USAGE_GENERIC_MOUSE  
    g_rid[0].dwFlags = RIDEV_INPUTSINK;
    g_rid[0].hwndTarget = g_hidden_window;
    
    if (!RegisterRawInputDevices(g_rid, 1, sizeof(g_rid[0]))) {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to register raw input device: error code %lu", error);
        return false;
    }
    
    LOG_INFO("Raw input device registered successfully");
    return true;
}

void MouseInput_ProcessRawInput(LPARAM lParam) {
    if (g_in_stabilizer_update) return;
    
    UINT dwSize = 0;
    UINT result = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    
    if (result != 0) {
        LOG_WARN("GetRawInputData size query failed: result=%u, error=%lu", result, GetLastError());
        return;
    }
    
    if (dwSize == 0) {
        LOG_DEBUG("Raw input data size is zero");
        return;
    }
    
    BYTE* lpb = malloc(dwSize);
    if (!lpb) {
        LOG_ERROR("Failed to allocate %u bytes for raw input data", dwSize);
        return;
    }
    
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
        LOG_WARN("GetRawInputData failed to read expected data size");
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