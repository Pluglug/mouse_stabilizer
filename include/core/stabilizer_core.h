#ifndef STABILIZER_CORE_H
#define STABILIZER_CORE_H

#include <windows.h>
#include <stdbool.h>
#include <math.h>

// Core stabilizer constants
#define UPDATE_INTERVAL_MS 8
#define DEFAULT_FOLLOW_STRENGTH 0.15f
#define DEFAULT_MIN_DISTANCE 0.5f
#define DEFAULT_DELAY_START_MS 150

// Easing types for smooth movement
typedef enum {
    EASE_LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT
} EaseType;

// 2D position structure
typedef struct {
    float x, y;
} MousePos;

// Main stabilizer state and configuration
typedef struct {
    MousePos target_pos;        // Target position from raw input
    MousePos current_pos;       // Current smoothed position
    float follow_strength;      // How quickly cursor follows target (0.05-1.0)
    float min_distance;         // Minimum distance to trigger movement
    EaseType ease_type;         // Easing function type
    bool dual_mode;             // Enable velocity-based adaptive following
    bool enabled;               // Whether stabilizer is active
    
    // Movement tracking
    float velocity;             // Current movement velocity
    DWORD last_update_time;     // Last velocity calculation time
    DWORD movement_start_time;  // When current movement started
    bool first_update;          // First update flag
    bool is_moving;             // Currently in motion
    
    // Delay and visual feedback
    DWORD delay_start_ms;       // Delay before following starts
    float target_show_distance; // Distance threshold for showing target pointer
    int target_size;            // Target pointer size
    int target_alpha;           // Target pointer transparency
    COLORREF target_color;      // Target pointer color
} SmoothStabilizer;

// Global stabilizer instance
extern SmoothStabilizer g_stabilizer;

// Core stabilizer functions
void StabilizerCore_Initialize(SmoothStabilizer* stabilizer);
void StabilizerCore_UpdatePosition(SmoothStabilizer* stabilizer);
void StabilizerCore_SetTargetPosition(SmoothStabilizer* stabilizer, float x, float y);
void StabilizerCore_AddMouseDelta(SmoothStabilizer* stabilizer, float dx, float dy);

// Utility functions
float StabilizerCore_ApplyEasing(float t, EaseType ease_type);
float StabilizerCore_CalculateDistance(MousePos a, MousePos b);
float StabilizerCore_CalculateVelocity(SmoothStabilizer* stabilizer, MousePos new_target);

#endif // STABILIZER_CORE_H