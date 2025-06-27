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

// Global target window handle
extern HWND g_target_window;

// Target pointer management
bool TargetPointer_CreateWindow(void);
void TargetPointer_UpdateWindow(void);
void TargetPointer_Show(bool show);
LRESULT CALLBACK TargetPointer_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // TARGET_POINTER_H