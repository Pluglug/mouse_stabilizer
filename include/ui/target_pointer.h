#ifndef TARGET_POINTER_H
#define TARGET_POINTER_H

#include <windows.h>
#include <stdbool.h>

// Target pointer constants
#define DRAW_INTERVAL_MS 16
#define DEFAULT_TARGET_SHOW_DISTANCE 5.0f
#define DEFAULT_POINTER_TYPE POINTER_CIRCLE
#define DEFAULT_TARGET_SIZE 8
#define DEFAULT_TARGET_ALPHA 180
#define DEFAULT_TARGET_ALWAYS_VISIBLE false
#define DEFAULT_EXCLUDE_FROM_CAPTURE true
#define DEFAULT_CAPTURE_COMPATIBILITY_MODE false

// Windows 10+ constant for excluding from capture
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

// Global target window handle
extern HWND g_target_window;

// Target pointer management
bool TargetPointer_CreateWindow(void);
void TargetPointer_UpdateWindow(void);
void TargetPointer_UpdateSettings(void);
void TargetPointer_Show(bool show);
LRESULT CALLBACK TargetPointer_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Screen capture exclusion functions
void TargetPointer_SetCaptureExclusion(bool exclude);
bool TargetPointer_IsCaptureExcluded(void);
void TargetPointer_UpdateCaptureSettings(void);

#endif // TARGET_POINTER_H