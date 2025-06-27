/**
 * Mouse Stabilizer - Core Engine
 * 
 * Implements paint-style following smoothing where the Windows cursor
 * smoothly follows a target position with configurable delay and easing.
 */

#include "mouse_stabilizer.h"

// Global application state
SmoothStabilizer g_stabilizer = {0};  // Main stabilizer instance
HWND g_hidden_window = NULL;          // Hidden window for message handling
HWND g_target_window = NULL;          // Overlay window for target pointer
NOTIFYICONDATA g_nid = {0};          // System tray icon data
bool g_running = true;                // Application running flag

void StabilizerCore_Initialize(SmoothStabilizer* stabilizer) {
    if (!stabilizer) {
        LOG_ERROR("StabilizerCore_Initialize: null stabilizer parameter");
        return;
    }
    
    POINT current_pos;
    if (!GetCursorPos(&current_pos)) {
        LOG_ERROR("Failed to get cursor position: error code %lu", GetLastError());
        // Use fallback position
        current_pos.x = 100;
        current_pos.y = 100;
    }
    
    stabilizer->target_pos.x = (float)current_pos.x;
    stabilizer->target_pos.y = (float)current_pos.y;
    stabilizer->current_pos.x = (float)current_pos.x;
    stabilizer->current_pos.y = (float)current_pos.y;
    
    stabilizer->follow_strength = DEFAULT_FOLLOW_STRENGTH;
    stabilizer->min_distance = DEFAULT_MIN_DISTANCE;
    stabilizer->ease_type = EASE_OUT;
    stabilizer->dual_mode = true;
    stabilizer->enabled = true;
    
    stabilizer->velocity = 0.0f;
    stabilizer->last_update_time = GetTickCount();
    stabilizer->movement_start_time = 0;
    stabilizer->first_update = true;
    stabilizer->is_moving = false;
    
    stabilizer->delay_start_ms = DEFAULT_DELAY_START_MS;
    stabilizer->target_show_distance = DEFAULT_TARGET_SHOW_DISTANCE;
    stabilizer->target_size = DEFAULT_TARGET_SIZE;
    stabilizer->target_alpha = DEFAULT_TARGET_ALPHA;
    stabilizer->target_color = RGB(255, 100, 100);
    stabilizer->pointer_type = DEFAULT_POINTER_TYPE;
    
    // Initialize capture exclusion settings
    stabilizer->exclude_from_capture = DEFAULT_EXCLUDE_FROM_CAPTURE;
    stabilizer->capture_compatibility_mode = DEFAULT_CAPTURE_COMPATIBILITY_MODE;
    
    Settings_WriteLog("Stabilizer initialized at position (%.1f, %.1f)", 
             stabilizer->current_pos.x, stabilizer->current_pos.y);
}

float StabilizerCore_ApplyEasing(float t, EaseType ease_type) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    
    switch (ease_type) {
        case EASE_LINEAR:
            return t;
        case EASE_IN:
            return t * t;
        case EASE_OUT:
            return 1.0f - (1.0f - t) * (1.0f - t);
        case EASE_IN_OUT:
            if (t < 0.5f) {
                return 2.0f * t * t;
            } else {
                return 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
            }
        default:
            return t;
    }
}

float StabilizerCore_CalculateDistance(MousePos a, MousePos b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

float StabilizerCore_CalculateVelocity(SmoothStabilizer* stabilizer, MousePos new_target) {
    DWORD current_time = GetTickCount();
    float dt = (current_time - stabilizer->last_update_time) / 1000.0f;
    
    if (dt <= 0.001f) return stabilizer->velocity;
    
    float distance = StabilizerCore_CalculateDistance(stabilizer->target_pos, new_target);
    float velocity = distance / dt;
    
    stabilizer->velocity = velocity * 0.7f + stabilizer->velocity * 0.3f;
    stabilizer->last_update_time = current_time;
    
    return stabilizer->velocity;
}

/**
 * Update cursor position with smooth following behavior
 * Core function that moves Windows cursor towards target with easing
 */
void StabilizerCore_UpdatePosition(SmoothStabilizer* stabilizer) {
    if (!stabilizer) {
        LOG_ERROR("StabilizerCore_UpdatePosition: null stabilizer parameter");
        return;
    }
    
    if (!stabilizer->enabled) return;
    
    float distance = StabilizerCore_CalculateDistance(stabilizer->current_pos, stabilizer->target_pos);
    
    if (distance < stabilizer->min_distance) {
        stabilizer->is_moving = false;
        return;
    }
    
    DWORD current_time = GetTickCount();
    
    if (!stabilizer->is_moving) {
        stabilizer->movement_start_time = current_time;
        stabilizer->is_moving = true;
    }
    
    // Implement delay start - wait before beginning to follow
    DWORD elapsed_since_start = current_time - stabilizer->movement_start_time;
    if (elapsed_since_start < stabilizer->delay_start_ms) {
        return;
    }
    
    // Calculate follow strength with optional velocity adaptation
    float follow_factor = stabilizer->follow_strength;
    if (stabilizer->dual_mode && stabilizer->velocity > 100.0f) {
        follow_factor = fminf(stabilizer->follow_strength * 3.0f, 0.8f);  // Faster following for quick movements
    }
    
    float eased_factor = StabilizerCore_ApplyEasing(follow_factor, stabilizer->ease_type);
    
    float dx = stabilizer->target_pos.x - stabilizer->current_pos.x;
    float dy = stabilizer->target_pos.y - stabilizer->current_pos.y;
    
    stabilizer->current_pos.x += dx * eased_factor;
    stabilizer->current_pos.y += dy * eased_factor;
    
    int new_x = (int)(stabilizer->current_pos.x + 0.5f);
    int new_y = (int)(stabilizer->current_pos.y + 0.5f);
    
    LOG_TRACE("Moving cursor to (%d, %d)", new_x, new_y);
    
    if (!SetCursorPos(new_x, new_y)) {
        LOG_WARN("Failed to set cursor position to (%d, %d): error code %lu", 
                 new_x, new_y, GetLastError());
    }
}

void StabilizerCore_SetTargetPosition(SmoothStabilizer* stabilizer, float x, float y) {
    if (!stabilizer->enabled) {
        SetCursorPos((int)x, (int)y);
        return;
    }
    
    if (stabilizer->first_update) {
        stabilizer->target_pos.x = x;
        stabilizer->target_pos.y = y;
        stabilizer->current_pos.x = x;
        stabilizer->current_pos.y = y;
        stabilizer->first_update = false;
        SetCursorPos((int)x, (int)y);
        return;
    }
    
    MousePos new_target = {x, y};
    StabilizerCore_CalculateVelocity(stabilizer, new_target);
    
    stabilizer->target_pos.x = x;
    stabilizer->target_pos.y = y;
}

/**
 * Process raw mouse movement delta from Windows Raw Input
 * Updates target position that the cursor will smoothly follow
 */
void StabilizerCore_AddMouseDelta(SmoothStabilizer* stabilizer, float dx, float dy) {
    if (!stabilizer) {
        LOG_ERROR("StabilizerCore_AddMouseDelta: null stabilizer parameter");
        return;
    }
    
    if (!stabilizer->enabled) {
        return;
    }
    
    LOG_DEBUG("Processing mouse delta: dx=%.1f, dy=%.1f", dx, dy);
    
    
    // Initialize positions on first update
    if (stabilizer->first_update) {
        POINT current_cursor;
        GetCursorPos(&current_cursor);
        stabilizer->target_pos.x = (float)current_cursor.x;
        stabilizer->target_pos.y = (float)current_cursor.y;
        stabilizer->current_pos.x = (float)current_cursor.x;
        stabilizer->current_pos.y = (float)current_cursor.y;
        stabilizer->first_update = false;
        return;
    }
    
    float new_x = stabilizer->target_pos.x + dx;
    float new_y = stabilizer->target_pos.y + dy;
    
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    if (new_x < 0) new_x = 0;
    if (new_y < 0) new_y = 0;
    if (new_x >= screen_width) new_x = screen_width - 1;
    if (new_y >= screen_height) new_y = screen_height - 1;
    
    
    stabilizer->target_pos.x = new_x;
    stabilizer->target_pos.y = new_y;
    
    StabilizerCore_CalculateVelocity(stabilizer, stabilizer->target_pos);
}