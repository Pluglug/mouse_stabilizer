#include "mouse_stabilizer.h"

MouseStabilizer g_stabilizer = {0};
HWND g_hidden_window = NULL;
bool g_running = true;

void InitializeStabilizer(MouseStabilizer* stabilizer) {
    stabilizer->head = 0;
    stabilizer->count = 0;
    stabilizer->smoothing_strength = DEFAULT_SMOOTHING_STRENGTH;
    stabilizer->threshold = DEFAULT_THRESHOLD;
    stabilizer->filter_type = FILTER_EXPONENTIAL;
    stabilizer->enabled = true;
    stabilizer->last_output = (MousePoint){0, 0, 0};
    stabilizer->velocity_x = 0.0f;
    stabilizer->velocity_y = 0.0f;
    stabilizer->accel_x = 0.0f;
    stabilizer->accel_y = 0.0f;
}

void AddMousePoint(MouseStabilizer* stabilizer, float x, float y) {
    DWORD current_time = GetTickCount();
    
    stabilizer->buffer[stabilizer->head] = (MousePoint){x, y, current_time};
    stabilizer->head = (stabilizer->head + 1) % MAX_BUFFER_SIZE;
    
    if (stabilizer->count < MAX_BUFFER_SIZE) {
        stabilizer->count++;
    }
}

void CalculateVelocityAcceleration(MouseStabilizer* stabilizer) {
    if (stabilizer->count < 2) return;
    
    int current_idx = (stabilizer->head - 1 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
    int prev_idx = (stabilizer->head - 2 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
    
    MousePoint current = stabilizer->buffer[current_idx];
    MousePoint prev = stabilizer->buffer[prev_idx];
    
    float dt = (current.timestamp - prev.timestamp) / 1000.0f;
    if (dt <= 0) dt = 0.001f;
    
    float new_vel_x = (current.x - prev.x) / dt;
    float new_vel_y = (current.y - prev.y) / dt;
    
    stabilizer->accel_x = (new_vel_x - stabilizer->velocity_x) / dt;
    stabilizer->accel_y = (new_vel_y - stabilizer->velocity_y) / dt;
    
    stabilizer->velocity_x = new_vel_x;
    stabilizer->velocity_y = new_vel_y;
}

bool IsIntentionalMovement(const MouseStabilizer* stabilizer) {
    float speed = sqrt(stabilizer->velocity_x * stabilizer->velocity_x + 
                      stabilizer->velocity_y * stabilizer->velocity_y);
    float acceleration = sqrt(stabilizer->accel_x * stabilizer->accel_x + 
                             stabilizer->accel_y * stabilizer->accel_y);
    
    return (speed > stabilizer->threshold * 10) || (acceleration > stabilizer->threshold * 20);
}

void ProcessMouseInput(const RAWMOUSE* mouse_data) {
    (void)mouse_data;
    if (!g_stabilizer.enabled) return;
    
    static POINT last_cursor_pos = {0, 0};
    POINT cursor_pos;
    GetCursorPos(&cursor_pos);
    
    if (cursor_pos.x == last_cursor_pos.x && cursor_pos.y == last_cursor_pos.y) {
        return;
    }
    
    AddMousePoint(&g_stabilizer, (float)cursor_pos.x, (float)cursor_pos.y);
    CalculateVelocityAcceleration(&g_stabilizer);
    
    if (IsIntentionalMovement(&g_stabilizer)) {
        WriteLog("Intentional movement detected, bypassing filter");
        last_cursor_pos = cursor_pos;
        return;
    }
    
    MousePoint filtered = ApplyFilter(&g_stabilizer);
    
    if (filtered.x != cursor_pos.x || filtered.y != cursor_pos.y) {
        SetCursorPos((int)filtered.x, (int)filtered.y);
        WriteLog("Applied filter: (%.1f, %.1f) -> (%.1f, %.1f)", 
                (float)cursor_pos.x, (float)cursor_pos.y, filtered.x, filtered.y);
    }
    
    last_cursor_pos.x = (int)filtered.x;
    last_cursor_pos.y = (int)filtered.y;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && g_stabilizer.enabled) {
        if (wParam == WM_MOUSEMOVE) {
            MSLLHOOKSTRUCT* mouse_struct = (MSLLHOOKSTRUCT*)lParam;
            
            AddMousePoint(&g_stabilizer, (float)mouse_struct->pt.x, (float)mouse_struct->pt.y);
            CalculateVelocityAcceleration(&g_stabilizer);
            
            if (!IsIntentionalMovement(&g_stabilizer)) {
                MousePoint filtered = ApplyFilter(&g_stabilizer);
                
                if (fabs(filtered.x - mouse_struct->pt.x) > 0.5f || 
                    fabs(filtered.y - mouse_struct->pt.y) > 0.5f) {
                    
                    SetCursorPos((int)filtered.x, (int)filtered.y);
                    return 1;
                }
            }
        }
    }
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}