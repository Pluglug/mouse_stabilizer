#ifndef SETTINGS_UI_H
#define SETTINGS_UI_H

#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>

// Settings window constants
#define SETTINGS_WINDOW_WIDTH 480
#define SETTINGS_WINDOW_HEIGHT 520
#define CONTROL_HEIGHT 28
#define CONTROL_SPACING 45
#define TAB_HEIGHT 35
#define LABEL_WIDTH 140
#define CONTROL_WIDTH 160
#define EDIT_WIDTH 60

// Control IDs for settings window
#define IDC_TAB_CONTROL     2000
#define IDC_FOLLOW_SLIDER   2001
#define IDC_FOLLOW_EDIT     2002
#define IDC_EASE_COMBO      2003
#define IDC_DELAY_SLIDER    2004
#define IDC_DELAY_EDIT      2005
#define IDC_DUAL_CHECK      2006
#define IDC_TARGET_DIST_SLIDER  2007
#define IDC_TARGET_DIST_EDIT    2008
#define IDC_TARGET_SIZE_SLIDER  2009
#define IDC_TARGET_SIZE_EDIT    2010
#define IDC_TARGET_ALPHA_SLIDER 2011
#define IDC_TARGET_ALPHA_EDIT   2012
#define IDC_ENABLE_CHECK        2013
#define IDC_LOG_LEVEL_COMBO     2014
#define IDC_PRESET_COMBO        2015
#define IDC_APPLY_PRESET        2016

// Tooltip control ID
#define IDC_TOOLTIP             2017

// Tab indices
#define TAB_BASIC       0
#define TAB_VISUAL      1
#define TAB_DEBUG       2

// Global settings window handle
extern HWND g_settings_window;
extern HFONT g_ui_font;
extern HWND g_tooltip;

// Settings UI functions
bool SettingsUI_Initialize(void);
void SettingsUI_ShowWindow(void);
void SettingsUI_HideWindow(void);
void SettingsUI_UpdateControls(void);
void SettingsUI_ApplySettings(void);
LRESULT CALLBACK SettingsUI_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Tab management
bool SettingsUI_CreateTabs(HWND hwnd);
bool SettingsUI_CreateBasicTab(HWND hwnd);
bool SettingsUI_CreateVisualTab(HWND hwnd);
bool SettingsUI_CreateDebugTab(HWND hwnd);
void SettingsUI_ShowTab(int tab_index);

// Helper function declaration
BOOL CALLBACK SettingsUI_ShowTabControls(HWND hwnd, LPARAM lParam);

// Control helpers
void SettingsUI_UpdateSliderAndEdit(int slider_id, int edit_id, float value, float min_val, float max_val);
float SettingsUI_GetSliderValue(int slider_id, float min_val, float max_val);
void SettingsUI_AddTooltip(HWND control, const char* text);
void SettingsUI_CreateFont(void);
void SettingsUI_ApplyFont(HWND control);

#endif // SETTINGS_UI_H