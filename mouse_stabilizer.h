#ifndef MOUSE_STABILIZER_H
#define MOUSE_STABILIZER_H

#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define UPDATE_INTERVAL_MS 8
#define DRAW_INTERVAL_MS 16
#define DEFAULT_FOLLOW_STRENGTH 0.15f
#define DEFAULT_MIN_DISTANCE 0.5f
#define DEFAULT_DELAY_START_MS 150
#define DEFAULT_TARGET_SHOW_DISTANCE 5.0f
#define DEFAULT_TARGET_SIZE 8
#define DEFAULT_TARGET_ALPHA 180
#define HOTKEY_ID 1
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)
#define WM_UPDATE_TIMER (WM_USER + 2)
#define WM_DRAW_TIMER (WM_USER + 3)
#define TIMER_ID 1
#define DRAW_TIMER_ID 2

typedef enum {
    EASE_LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT
} EaseType;

typedef struct {
    float x, y;
} MousePos;

typedef struct {
    MousePos target_pos;
    MousePos current_pos;
    float follow_strength;
    float min_distance;
    EaseType ease_type;
    bool dual_mode;
    bool enabled;
    
    float velocity;
    DWORD last_update_time;
    DWORD movement_start_time;
    bool first_update;
    bool is_moving;
    
    DWORD delay_start_ms;
    float target_show_distance;
    int target_size;
    int target_alpha;
    COLORREF target_color;
} SmoothStabilizer;

extern SmoothStabilizer g_stabilizer;
extern HWND g_hidden_window;
extern HWND g_target_window;
extern NOTIFYICONDATA g_nid;
extern bool g_running;

void InitializeStabilizer(SmoothStabilizer* stabilizer);
float ApplyEasing(float t, EaseType ease_type);
float CalculateDistance(MousePos a, MousePos b);
float CalculateVelocity(SmoothStabilizer* stabilizer, MousePos new_target);
void UpdateSmoothPosition(SmoothStabilizer* stabilizer);
void SetTargetPosition(SmoothStabilizer* stabilizer, float x, float y);
void AddMouseDelta(SmoothStabilizer* stabilizer, float dx, float dy);
bool RegisterRawInput(void);
void ProcessRawInput(LPARAM lParam);
bool CreateTargetWindow(void);
void UpdateTargetWindow(void);
void ShowTargetPointer(bool show);
LRESULT CALLBACK TargetWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool CreateTrayIcon(HWND hwnd);
void ShowContextMenu(HWND hwnd);
void ToggleStabilizer(void);
void UpdateTrayIcon(void);
void WriteLog(const char* format, ...);
void LoadSettings(void);
void SaveSettings(void);

#endif