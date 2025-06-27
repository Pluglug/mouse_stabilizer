#ifndef MOUSE_STABILIZER_H
#define MOUSE_STABILIZER_H

#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define MAX_BUFFER_SIZE 32
#define DEFAULT_SMOOTHING_STRENGTH 0.3f
#define DEFAULT_THRESHOLD 5.0f
#define HOTKEY_ID 1
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

typedef enum {
    FILTER_MOVING_AVERAGE,
    FILTER_EXPONENTIAL,
    FILTER_KALMAN
} FilterType;

typedef struct {
    float x, y;
    DWORD timestamp;
} MousePoint;

typedef struct {
    MousePoint buffer[MAX_BUFFER_SIZE];
    int head;
    int count;
    float smoothing_strength;
    float threshold;
    FilterType filter_type;
    bool enabled;
    MousePoint last_output;
    float velocity_x, velocity_y;
    float accel_x, accel_y;
} MouseStabilizer;

typedef struct {
    float x, y;
    float p_x, p_y;
    float q;
    float r;
} KalmanFilter;

extern MouseStabilizer g_stabilizer;
extern KalmanFilter g_kalman_x, g_kalman_y;
extern HWND g_hidden_window;
extern NOTIFYICONDATA g_nid;
extern bool g_running;

void InitializeStabilizer(MouseStabilizer* stabilizer);
void InitializeKalman(KalmanFilter* kf, float q, float r);
float KalmanUpdate(KalmanFilter* kf, float measurement);
void AddMousePoint(MouseStabilizer* stabilizer, float x, float y);
void CalculateVelocityAcceleration(MouseStabilizer* stabilizer);
bool IsIntentionalMovement(const MouseStabilizer* stabilizer);
MousePoint ApplyFilter(const MouseStabilizer* stabilizer);
MousePoint MovingAverageFilter(const MouseStabilizer* stabilizer);
MousePoint ExponentialFilter(const MouseStabilizer* stabilizer);
MousePoint KalmanFilter_Apply(const MouseStabilizer* stabilizer);
void ProcessMouseInput(const RAWMOUSE* mouse_data);
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