/**
 * Settings UI - Advanced Configuration Window
 * 
 * Provides a tabbed interface for detailed configuration of stabilizer
 * settings with real-time updates and intuitive controls.
 */

#include "mouse_stabilizer.h"
#include <windowsx.h>

// Global settings window handle
HWND g_settings_window = NULL;
static HWND g_tab_control = NULL;
static int g_current_tab = 0;
static bool g_updating_controls = false;  // Flag to prevent feedback loops

bool SettingsUI_Initialize(void) {
    const char* class_name = "MouseStabilizerSettings";
    WNDCLASS wc = {0};
    
    wc.lpfnWndProc = SettingsUI_WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = class_name;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    
    if (!RegisterClass(&wc)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            LOG_ERROR("Failed to register settings window class: error code %lu", error);
            return false;
        }
    }
    
    // Initialize common controls for tabs and trackbars
    INITCOMMONCONTROLSEX icc = {0};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_UPDOWN_CLASS;
    
    if (!InitCommonControlsEx(&icc)) {
        LOG_WARN("Failed to initialize common controls: error code %lu", GetLastError());
    }
    
    LOG_INFO("Settings UI initialized successfully");
    return true;
}

void SettingsUI_ShowWindow(void) {
    LOG_DEBUG("SettingsUI_ShowWindow called");
    
    if (!g_settings_window) {
        LOG_DEBUG("Creating new settings window");
        
        // Create the window if it doesn't exist
        g_settings_window = CreateWindowEx(
            WS_EX_DLGMODALFRAME,
            "MouseStabilizerSettings",
            "Mouse Stabilizer Settings",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT,
            SETTINGS_WINDOW_WIDTH, SETTINGS_WINDOW_HEIGHT,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );
        
        if (!g_settings_window) {
            DWORD error = GetLastError();
            LOG_ERROR("Failed to create settings window: error code %lu", error);
            return;
        }
        
        LOG_DEBUG("Settings window created successfully, handle: %p", (void*)g_settings_window);
        
        if (!SettingsUI_CreateTabs(g_settings_window)) {
            LOG_ERROR("Failed to create tabs, destroying window");
            DestroyWindow(g_settings_window);
            g_settings_window = NULL;
            return;
        }
        
        LOG_DEBUG("Tabs created successfully, updating controls");
        SettingsUI_UpdateControls();
        LOG_DEBUG("Controls updated successfully");
    }
    
    LOG_DEBUG("Showing settings window");
    ShowWindow(g_settings_window, SW_SHOW);
    SetForegroundWindow(g_settings_window);
    LOG_DEBUG("Settings window shown successfully");
}

void SettingsUI_HideWindow(void) {
    if (g_settings_window) {
        ShowWindow(g_settings_window, SW_HIDE);
        LOG_DEBUG("Settings window hidden");
    }
}

bool SettingsUI_CreateTabs(HWND hwnd) {
    LOG_DEBUG("Creating tab control");
    
    // Create tab control
    g_tab_control = CreateWindowEx(0, WC_TABCONTROL, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        10, 10, SETTINGS_WINDOW_WIDTH - 40, SETTINGS_WINDOW_HEIGHT - 80,
        hwnd, (HMENU)IDC_TAB_CONTROL, GetModuleHandle(NULL), NULL);
    
    if (!g_tab_control) {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to create tab control: error code %lu", error);
        return false;
    }
    
    LOG_DEBUG("Tab control created, handle: %p", (void*)g_tab_control);
    
    // Add tabs
    TCITEM tie = {0};
    tie.mask = TCIF_TEXT;
    
    tie.pszText = "Basic Settings";
    if (TabCtrl_InsertItem(g_tab_control, TAB_BASIC, &tie) == -1) {
        LOG_ERROR("Failed to insert Basic tab");
        return false;
    }
    
    tie.pszText = "Visual";
    if (TabCtrl_InsertItem(g_tab_control, TAB_VISUAL, &tie) == -1) {
        LOG_ERROR("Failed to insert Visual tab");
        return false;
    }
    
    tie.pszText = "Debug";
    if (TabCtrl_InsertItem(g_tab_control, TAB_DEBUG, &tie) == -1) {
        LOG_ERROR("Failed to insert Debug tab");
        return false;
    }
    
    LOG_DEBUG("All tabs inserted successfully");
    
    // Create tab content
    if (!SettingsUI_CreateBasicTab(hwnd)) {
        LOG_ERROR("Failed to create Basic tab content");
        return false;
    }
    
    if (!SettingsUI_CreateVisualTab(hwnd)) {
        LOG_ERROR("Failed to create Visual tab content");
        return false;
    }
    
    if (!SettingsUI_CreateDebugTab(hwnd)) {
        LOG_ERROR("Failed to create Debug tab content");
        return false;
    }
    
    LOG_DEBUG("All tab content created successfully");
    
    SettingsUI_ShowTab(TAB_BASIC);
    LOG_DEBUG("Basic tab shown as default");
    
    return true;
}

bool SettingsUI_CreateBasicTab(HWND hwnd) {
    LOG_DEBUG("Creating Basic tab controls");
    
    int y_pos = 50;
    HWND parent = hwnd;
    HWND control;
    
    // Follow Strength
    control = CreateWindow("STATIC", "Follow Strength:", WS_CHILD | WS_VISIBLE,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Follow Strength label");
        return false;
    }
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_TOOLTIPS,
        150, y_pos, 150, 25, parent, (HMENU)IDC_FOLLOW_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Follow Strength slider");
        return false;
    }
    
    control = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        310, y_pos, 50, 20, parent, (HMENU)IDC_FOLLOW_EDIT,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Follow Strength edit");
        return false;
    }
    
    y_pos += CONTROL_SPACING;
    
    // Ease Type
    control = CreateWindow("STATIC", "Ease Type:", WS_CHILD | WS_VISIBLE,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Ease Type label");
        return false;
    }
    
    HWND ease_combo = CreateWindow("COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        150, y_pos, 150, 100, parent, (HMENU)IDC_EASE_COMBO,
        GetModuleHandle(NULL), NULL);
    if (!ease_combo) {
        LOG_ERROR("Failed to create Ease Type combo");
        return false;
    }
    
    // Populate ease combo
    ComboBox_AddString(ease_combo, "Linear");
    ComboBox_AddString(ease_combo, "Ease In");
    ComboBox_AddString(ease_combo, "Ease Out");
    ComboBox_AddString(ease_combo, "Ease In-Out");
    
    y_pos += CONTROL_SPACING;
    
    // Delay Start
    control = CreateWindow("STATIC", "Delay Start (ms):", WS_CHILD | WS_VISIBLE,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Delay Start label");
        return false;
    }
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_TOOLTIPS,
        150, y_pos, 150, 25, parent, (HMENU)IDC_DELAY_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Delay Start slider");
        return false;
    }
    
    control = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        310, y_pos, 50, 20, parent, (HMENU)IDC_DELAY_EDIT,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Delay Start edit");
        return false;
    }
    
    y_pos += CONTROL_SPACING;
    
    // Dual Mode
    control = CreateWindow("BUTTON", "Enable Dual Mode (velocity adaptive)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, y_pos, 250, 20, parent, (HMENU)IDC_DUAL_CHECK,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Dual Mode checkbox");
        return false;
    }
    
    LOG_DEBUG("Basic tab controls created successfully");
    return true;
}

bool SettingsUI_CreateVisualTab(HWND hwnd) {
    LOG_DEBUG("Creating Visual tab controls");
    
    // Visual controls will be hidden initially
    int y_pos = 50;
    HWND parent = hwnd;
    HWND control;
    
    // Target Distance
    control = CreateWindow("STATIC", "Target Distance:", WS_CHILD,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Distance label");
        return false;
    }
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | TBS_HORZ | TBS_TOOLTIPS,
        150, y_pos, 150, 25, parent, (HMENU)IDC_TARGET_DIST_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Distance slider");
        return false;
    }
    
    control = CreateWindow("EDIT", NULL, WS_CHILD | WS_BORDER | ES_NUMBER,
        310, y_pos, 50, 20, parent, (HMENU)IDC_TARGET_DIST_EDIT,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Distance edit");
        return false;
    }
    
    y_pos += CONTROL_SPACING;
    
    // Target Size  
    control = CreateWindow("STATIC", "Target Size:", WS_CHILD,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Size label");
        return false;
    }
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | TBS_HORZ | TBS_TOOLTIPS,
        150, y_pos, 150, 25, parent, (HMENU)IDC_TARGET_SIZE_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Size slider");
        return false;
    }
    
    control = CreateWindow("EDIT", NULL, WS_CHILD | WS_BORDER | ES_NUMBER,
        310, y_pos, 50, 20, parent, (HMENU)IDC_TARGET_SIZE_EDIT,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Size edit");
        return false;
    }
    
    y_pos += CONTROL_SPACING;
    
    // Target Alpha
    control = CreateWindow("STATIC", "Target Alpha:", WS_CHILD,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Alpha label");
        return false;
    }
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | TBS_HORZ | TBS_TOOLTIPS,
        150, y_pos, 150, 25, parent, (HMENU)IDC_TARGET_ALPHA_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Alpha slider");
        return false;
    }
    
    control = CreateWindow("EDIT", NULL, WS_CHILD | WS_BORDER | ES_NUMBER,
        310, y_pos, 50, 20, parent, (HMENU)IDC_TARGET_ALPHA_EDIT,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Alpha edit");
        return false;
    }
    
    LOG_DEBUG("Visual tab controls created successfully");
    return true;
}

bool SettingsUI_CreateDebugTab(HWND hwnd) {
    LOG_DEBUG("Creating Debug tab controls");
    
    int y_pos = 50;
    HWND parent = hwnd;
    HWND control;
    
    // Log Level
    control = CreateWindow("STATIC", "Log Level:", WS_CHILD,
        20, y_pos, 120, 20, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Log Level label");
        return false;
    }
    
    HWND log_combo = CreateWindow("COMBOBOX", NULL,
        WS_CHILD | CBS_DROPDOWNLIST,
        150, y_pos, 150, 120, parent, (HMENU)IDC_LOG_LEVEL_COMBO,
        GetModuleHandle(NULL), NULL);
    if (!log_combo) {
        LOG_ERROR("Failed to create Log Level combo");
        return false;
    }
    
    // Populate log level combo
    ComboBox_AddString(log_combo, "ERROR");
    ComboBox_AddString(log_combo, "WARN");
    ComboBox_AddString(log_combo, "INFO");
    ComboBox_AddString(log_combo, "DEBUG");
    ComboBox_AddString(log_combo, "TRACE");
    
    LOG_DEBUG("Debug tab controls created successfully");
    return true;
}

// Helper function for control visibility
BOOL CALLBACK SettingsUI_ShowTabControls(HWND hwnd, LPARAM lParam) {
    if (hwnd == g_tab_control) {
        return TRUE; // Skip the tab control itself
    }
    
    int tab = (int)lParam;
    int id = GetDlgCtrlID(hwnd);
    bool should_show = false;
    
    // Determine if control should be visible for this tab
    if (tab == TAB_BASIC && id >= IDC_FOLLOW_SLIDER && id <= IDC_DUAL_CHECK) {
        should_show = true;
    } else if (tab == TAB_VISUAL && id >= IDC_TARGET_DIST_SLIDER && id <= IDC_TARGET_ALPHA_EDIT) {
        should_show = true;
    } else if (tab == TAB_DEBUG && id >= IDC_LOG_LEVEL_COMBO) {
        should_show = true;
    }
    
    ShowWindow(hwnd, should_show ? SW_SHOW : SW_HIDE);
    return TRUE;
}

void SettingsUI_ShowTab(int tab_index) {
    // Update visibility for all child controls
    EnumChildWindows(g_settings_window, SettingsUI_ShowTabControls, (LPARAM)tab_index);
    g_current_tab = tab_index;
}

LRESULT CALLBACK SettingsUI_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            SettingsUI_HideWindow();
            return 0;
            
        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->idFrom == IDC_TAB_CONTROL && pnmh->code == TCN_SELCHANGE) {
                int tab = TabCtrl_GetCurSel(g_tab_control);
                SettingsUI_ShowTab(tab);
            }
            break;
        }
        
        case WM_COMMAND: {
            int code = HIWORD(wParam);
            
            // Handle real-time updates - but avoid feedback loops
            if ((code == CBN_SELCHANGE || code == BN_CLICKED || code == EN_CHANGE) && !g_updating_controls) {
                LOG_DEBUG("Control change detected, applying settings");
                SettingsUI_ApplySettings();
                Settings_Save();
            } else if (g_updating_controls) {
                LOG_DEBUG("Control change ignored during control update");
            }
            break;
        }
        
        case WM_HSCROLL: {
            // Handle slider changes - but avoid feedback loops
            if (!g_updating_controls) {
                LOG_DEBUG("Slider change detected, applying settings");
                SettingsUI_ApplySettings();
                Settings_Save();
            } else {
                LOG_DEBUG("Slider change ignored during control update");
            }
            break;
        }
        
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

void SettingsUI_UpdateControls(void) {
    if (!g_settings_window) {
        LOG_DEBUG("Settings window is NULL, skipping control update");
        return;
    }
    
    if (g_updating_controls) {
        LOG_DEBUG("Already updating controls, skipping to prevent loop");
        return;
    }
    
    g_updating_controls = true;
    LOG_DEBUG("Updating controls with current settings");
    
    char buffer[32];
    
    // Update Follow Strength
    HWND slider = GetDlgItem(g_settings_window, IDC_FOLLOW_SLIDER);
    if (slider) {
        SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(5, 100));  // 0.05 to 1.0
        SendMessage(slider, TBM_SETPOS, TRUE, (LPARAM)(g_stabilizer.follow_strength * 100));
        LOG_DEBUG("Follow strength slider updated: %.2f", g_stabilizer.follow_strength);
    } else {
        LOG_WARN("Follow strength slider not found");
    }
    
    HWND edit = GetDlgItem(g_settings_window, IDC_FOLLOW_EDIT);
    if (edit) {
        sprintf_s(buffer, sizeof(buffer), "%.2f", g_stabilizer.follow_strength);
        SetWindowText(edit, buffer);
        LOG_DEBUG("Follow strength edit updated: %s", buffer);
    } else {
        LOG_WARN("Follow strength edit not found");
    }
    
    // Update Ease Type
    HWND combo = GetDlgItem(g_settings_window, IDC_EASE_COMBO);
    if (combo) {
        ComboBox_SetCurSel(combo, g_stabilizer.ease_type);
    }
    
    // Update Delay
    slider = GetDlgItem(g_settings_window, IDC_DELAY_SLIDER);
    if (slider) {
        SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 500));
        SendMessage(slider, TBM_SETPOS, TRUE, g_stabilizer.delay_start_ms);
    }
    
    edit = GetDlgItem(g_settings_window, IDC_DELAY_EDIT);
    if (edit) {
        sprintf_s(buffer, sizeof(buffer), "%lu", (unsigned long)g_stabilizer.delay_start_ms);
        SetWindowText(edit, buffer);
    }
    
    // Update Dual Mode
    HWND check = GetDlgItem(g_settings_window, IDC_DUAL_CHECK);
    if (check) {
        Button_SetCheck(check, g_stabilizer.dual_mode ? BST_CHECKED : BST_UNCHECKED);
    }
    
    // Update Log Level combo
    combo = GetDlgItem(g_settings_window, IDC_LOG_LEVEL_COMBO);
    if (combo) {
        ComboBox_SetCurSel(combo, Settings_GetLogLevel());
    }
    
    LOG_DEBUG("Settings controls updated");
    g_updating_controls = false;
}

void SettingsUI_ApplySettings(void) {
    if (!g_settings_window) return;
    
    char buffer[64];
    
    // Apply Follow Strength - check edit field first, then slider
    HWND edit = GetDlgItem(g_settings_window, IDC_FOLLOW_EDIT);
    if (edit && GetWindowText(edit, buffer, sizeof(buffer)) > 0) {
        float value = (float)atof(buffer);
        if (value >= 0.05f && value <= 1.0f) {
            g_stabilizer.follow_strength = value;
        }
    } else {
        HWND slider = GetDlgItem(g_settings_window, IDC_FOLLOW_SLIDER);
        if (slider) {
            int pos = (int)SendMessage(slider, TBM_GETPOS, 0, 0);
            g_stabilizer.follow_strength = pos / 100.0f;
        }
    }
    
    // Apply Ease Type
    HWND combo = GetDlgItem(g_settings_window, IDC_EASE_COMBO);
    if (combo) {
        int sel = ComboBox_GetCurSel(combo);
        if (sel >= 0 && sel <= 3) {
            g_stabilizer.ease_type = (EaseType)sel;
        }
    }
    
    // Apply Delay - check edit field first, then slider
    edit = GetDlgItem(g_settings_window, IDC_DELAY_EDIT);
    if (edit && GetWindowText(edit, buffer, sizeof(buffer)) > 0) {
        int value = atoi(buffer);
        if (value >= 0 && value <= 500) {
            g_stabilizer.delay_start_ms = (DWORD)value;
        }
    } else {
        HWND slider = GetDlgItem(g_settings_window, IDC_DELAY_SLIDER);
        if (slider) {
            g_stabilizer.delay_start_ms = (DWORD)SendMessage(slider, TBM_GETPOS, 0, 0);
        }
    }
    
    // Apply Dual Mode
    HWND check = GetDlgItem(g_settings_window, IDC_DUAL_CHECK);
    if (check) {
        g_stabilizer.dual_mode = (Button_GetCheck(check) == BST_CHECKED);
    }
    
    // Apply Log Level
    combo = GetDlgItem(g_settings_window, IDC_LOG_LEVEL_COMBO);
    if (combo) {
        int sel = ComboBox_GetCurSel(combo);
        if (sel >= 0 && sel <= 4) {
            Settings_SetLogLevel((LogLevel)sel);
        }
    }
    
    LOG_DEBUG("Settings applied from UI controls");
}