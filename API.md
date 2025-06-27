# Mouse Stabilizer API Reference

## Core Functions

### StabilizerCore_Initialize()
```c
void StabilizerCore_Initialize(SmoothStabilizer* stabilizer);
```
Initializes stabilizer with default settings and current cursor position.

### StabilizerCore_UpdatePosition()
```c
void StabilizerCore_UpdatePosition(SmoothStabilizer* stabilizer);
```
Main update function called by timer. Smoothly moves cursor towards target.

### StabilizerCore_AddMouseDelta()
```c
void StabilizerCore_AddMouseDelta(SmoothStabilizer* stabilizer, float dx, float dy);
```
Processes raw mouse movement delta and updates target position.

## Configuration

### Settings_Load() / Settings_Save()
```c
void Settings_Load(void);
void Settings_Save(void);
```
Load/save configuration from/to `mouse_stabilizer.ini`.

### Settings_WriteLog()
```c
void Settings_WriteLog(const char* format, ...);
```
Write timestamped log entry to `mouse_stabilizer.log`.

## Key Data Structures

### SmoothStabilizer
Main configuration and state structure:
- `target_pos` - Raw mouse position
- `current_pos` - Smoothed cursor position  
- `follow_strength` - Smoothing speed (0.05-1.0)
- `ease_type` - Easing curve type
- `delay_start_ms` - Delay before following (0-500ms)
- `dual_mode` - Velocity-adaptive following

### EaseType
Smoothing curve options:
- `EASE_LINEAR` - Constant speed
- `EASE_IN` - Slow start, fast end
- `EASE_OUT` - Fast start, slow end  
- `EASE_IN_OUT` - Smooth acceleration/deceleration

## Constants

### Timing
- `UPDATE_INTERVAL_MS` (8) - Core update frequency
- `DRAW_INTERVAL_MS` (16) - Target pointer refresh rate

### Defaults  
- `DEFAULT_FOLLOW_STRENGTH` (0.15) - Balanced smoothing
- `DEFAULT_DELAY_START_MS` (150) - Medium delay
- `DEFAULT_TARGET_SHOW_DISTANCE` (5.0) - Target visibility threshold

## Usage Example

```c
// Initialize
StabilizerCore_Initialize(&g_stabilizer);
g_stabilizer.follow_strength = 0.1f;
g_stabilizer.ease_type = EASE_IN_OUT;

// Process mouse input (called from Raw Input handler)
StabilizerCore_AddMouseDelta(&g_stabilizer, dx, dy);

// Update cursor position (called from timer)
StabilizerCore_UpdatePosition(&g_stabilizer);
```