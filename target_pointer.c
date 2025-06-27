/**
 * Target Pointer - Visual Feedback Overlay
 * 
 * Creates a semi-transparent red circle overlay that shows the actual
 * mouse position (target) that the cursor is smoothly following towards.
 */

#include "mouse_stabilizer.h"

static bool g_target_visible = false;  // Current visibility state
static DWORD g_last_draw_time = 0;     // Throttle drawing updates

bool TargetPointer_CreateWindow(void) {
    const char* class_name = "MouseStabilizerTarget";
    WNDCLASS wc = {0};
    
    wc.lpfnWndProc = TargetPointer_WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = class_name;
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    
    if (!RegisterClass(&wc)) {
        DWORD error = GetLastError();
        if (error == ERROR_CLASS_ALREADY_EXISTS) {
            LOG_DEBUG("Target window class already registered");
        } else {
            LOG_ERROR("Failed to register target window class: error code %lu", error);
            return false;
        }
    }
    
    g_target_window = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        class_name, "Target Pointer",
        WS_POPUP,
        0, 0, g_stabilizer.target_size * 3, g_stabilizer.target_size * 3,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!g_target_window) {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to create target window: error code %lu", error);
        return false;
    }
    
    // Set transparent background using magenta as the transparent color
    if (!SetLayeredWindowAttributes(g_target_window, RGB(255, 0, 255), g_stabilizer.target_alpha, LWA_COLORKEY | LWA_ALPHA)) {
        DWORD error = GetLastError();
        LOG_WARN("Failed to set layered window attributes: error code %lu", error);
        // Continue anyway, window will still function
    }
    
    LOG_INFO("Target window created successfully");
    return true;
}

void TargetPointer_UpdateWindow(void) {
    if (!g_target_window || !g_stabilizer.enabled) return;
    
    DWORD current_time = GetTickCount();
    if (current_time - g_last_draw_time < DRAW_INTERVAL_MS) return;
    g_last_draw_time = current_time;
    
    float distance = StabilizerCore_CalculateDistance(g_stabilizer.target_pos, g_stabilizer.current_pos);
    bool should_show = distance >= g_stabilizer.target_show_distance;
    
    if (should_show != g_target_visible) {
        TargetPointer_Show(should_show);
    }
    
    if (g_target_visible) {
        int size = g_stabilizer.target_size * 3;
        int x = (int)(g_stabilizer.target_pos.x - size / 2);
        int y = (int)(g_stabilizer.target_pos.y - size / 2);
        
        SetWindowPos(g_target_window, HWND_TOPMOST, x, y, size, size, 
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        
        InvalidateRect(g_target_window, NULL, TRUE);
    }
}

void TargetPointer_UpdateSettings(void) {
    if (!g_target_window) return;
    
    // Update layered window attributes for transparency
    SetLayeredWindowAttributes(g_target_window, RGB(255, 0, 255), g_stabilizer.target_alpha, LWA_COLORKEY | LWA_ALPHA);
    
    // Update window size if currently visible
    if (g_target_visible) {
        int size = g_stabilizer.target_size * 3;
        int x = (int)(g_stabilizer.target_pos.x - size / 2);
        int y = (int)(g_stabilizer.target_pos.y - size / 2);
        
        SetWindowPos(g_target_window, HWND_TOPMOST, x, y, size, size, 
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    
    // Force repaint to reflect changes
    InvalidateRect(g_target_window, NULL, TRUE);
    UpdateWindow(g_target_window);
    
    LOG_DEBUG("Target pointer settings updated - Size: %d, Alpha: %d", 
              g_stabilizer.target_size, g_stabilizer.target_alpha);
}

void TargetPointer_Show(bool show) {
    if (!g_target_window) return;
    
    if (show && !g_target_visible) {
        ShowWindow(g_target_window, SW_SHOWNOACTIVATE);
        g_target_visible = true;
        Settings_WriteLog("Target pointer shown");
    } else if (!show && g_target_visible) {
        ShowWindow(g_target_window, SW_HIDE);
        g_target_visible = false;
        Settings_WriteLog("Target pointer hidden");
    }
}

LRESULT CALLBACK TargetPointer_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Fill background with transparent color (magenta)
            HBRUSH transparent_brush = CreateSolidBrush(RGB(255, 0, 255));
            FillRect(hdc, &rect, transparent_brush);
            DeleteObject(transparent_brush);
            
            int center_x = rect.right / 2;
            int center_y = rect.bottom / 2;
            int size = g_stabilizer.target_size;
            
            HBRUSH brush = CreateSolidBrush(g_stabilizer.target_color);
            HPEN pen = CreatePen(PS_SOLID, 2, g_stabilizer.target_color);
            
            SelectObject(hdc, brush);
            SelectObject(hdc, pen);
            
            if (g_stabilizer.pointer_type == POINTER_CIRCLE) {
                // Draw circle pointer
                Ellipse(hdc, 
                       center_x - size, center_y - size,
                       center_x + size, center_y + size);
            } else if (g_stabilizer.pointer_type == POINTER_CROSS) {
                // Draw cross pointer
                MoveToEx(hdc, center_x - size, center_y, NULL);
                LineTo(hdc, center_x + size, center_y);
                MoveToEx(hdc, center_x, center_y - size, NULL);
                LineTo(hdc, center_x, center_y + size);
            }
            
            DeleteObject(brush);
            DeleteObject(pen);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}