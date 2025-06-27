#ifndef HOTKEY_H
#define HOTKEY_H

#include <windows.h>
#include <stdbool.h>

// Hotkey management and window procedure functions

/**
 * Toggle the stabilizer enabled/disabled state
 */
void Hotkey_ToggleStabilizer(void);

/**
 * Main window procedure for handling hotkeys and system messages
 * @param hwnd Window handle
 * @param uMsg Message identifier
 * @param wParam Additional message info
 * @param lParam Additional message info
 * @return Message handling result
 */
LRESULT CALLBACK Hotkey_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // HOTKEY_H