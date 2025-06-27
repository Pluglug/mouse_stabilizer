#ifndef MOUSE_INPUT_H
#define MOUSE_INPUT_H

#include <windows.h>
#include <stdbool.h>

// Global window handles
extern HWND g_hidden_window;
extern bool g_running;

// Mouse input processing functions
bool MouseInput_RegisterRawInput(void);
void MouseInput_ProcessRawInput(LPARAM lParam);
LRESULT CALLBACK MouseInput_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseInput_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Hotkey and window management
void MouseInput_ToggleStabilizer(void);

#endif // MOUSE_INPUT_H