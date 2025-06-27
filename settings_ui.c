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
HFONT g_ui_font = NULL;
HWND g_tooltip = NULL;
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
    
    // Initialize common controls for tabs, trackbars, and tooltips
    INITCOMMONCONTROLSEX icc = {0};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_UPDOWN_CLASS | ICC_WIN95_CLASSES;
    
    if (!InitCommonControlsEx(&icc)) {
        LOG_WARN("Failed to initialize common controls: error code %lu", GetLastError());
    }
    
    // Create modern UI font
    SettingsUI_CreateFont();
    
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

void SettingsUI_CreateFont(void) {
    if (g_ui_font) {
        DeleteObject(g_ui_font);
    }
    
    // Create modern Segoe UI font
    g_ui_font = CreateFont(
        -14,                    // Height (-14 for 10.5pt at 96 DPI)
        0,                      // Width (0 = default)
        0,                      // Escapement
        0,                      // Orientation
        FW_NORMAL,              // Weight
        FALSE,                  // Italic
        FALSE,                  // Underline
        FALSE,                  // StrikeOut
        DEFAULT_CHARSET,        // CharSet
        OUT_DEFAULT_PRECIS,     // OutputPrecision
        CLIP_DEFAULT_PRECIS,    // ClipPrecision
        CLEARTYPE_QUALITY,      // Quality (ClearType for modern look)
        DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
        "Segoe UI"              // Font name
    );
    
    if (!g_ui_font) {
        LOG_WARN("Failed to create Segoe UI font, using default");
        g_ui_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }
}

void SettingsUI_ApplyFont(HWND control) {
    if (g_ui_font && control) {
        SendMessage(control, WM_SETFONT, (WPARAM)g_ui_font, TRUE);
    }
}

void SettingsUI_AddTooltip(HWND control, const char* text) {
    if (!g_tooltip || !control || !text) return;
    
    TOOLINFO ti = {0};
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = g_settings_window;
    ti.uId = (UINT_PTR)control;
    ti.lpszText = (LPSTR)text;
    
    SendMessage(g_tooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void SettingsUI_HideWindow(void) {
    if (g_settings_window) {
        ShowWindow(g_settings_window, SW_HIDE);
        LOG_DEBUG("Settings window hidden");
    }
}

bool SettingsUI_CreateTabs(HWND hwnd) {
    LOG_DEBUG("Creating tab control");
    
    // Create tooltip control first
    g_tooltip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwnd, (HMENU)IDC_TOOLTIP, GetModuleHandle(NULL), NULL);
    
    if (!g_tooltip) {
        LOG_WARN("Failed to create tooltip control");
    }
    
    // Create tab control
    g_tab_control = CreateWindowEx(0, WC_TABCONTROL, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        15, 15, SETTINGS_WINDOW_WIDTH - 50, SETTINGS_WINDOW_HEIGHT - 90,
        hwnd, (HMENU)IDC_TAB_CONTROL, GetModuleHandle(NULL), NULL);
    
    if (!g_tab_control) {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to create tab control: error code %lu", error);
        return false;
    }
    
    // Apply modern font to tab control
    SettingsUI_ApplyFont(g_tab_control);
    
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
    
    int y_pos = 55;
    int x_label = 30;
    int x_control = x_label + LABEL_WIDTH + 10;
    HWND parent = hwnd;
    HWND control;
    
    // Enable/Disable Stabilizer - Prominent at top
    control = CreateWindow("BUTTON", "Enable Mouse Stabilizer",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE,
        x_label, y_pos, LABEL_WIDTH + CONTROL_WIDTH + 10, 35,
        parent, (HMENU)IDC_ENABLE_CHECK, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Enable checkbox");
        return false;
    }
    SettingsUI_ApplyFont(control);
    SettingsUI_AddTooltip(control, "Toggle mouse stabilization on/off");
    
    y_pos += 50;
    
    // Follow Strength
    control = CreateWindow("STATIC", "Follow Strength:", WS_CHILD | WS_VISIBLE,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Follow Strength label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_TOOLTIPS | TBS_ENABLESELRANGE,
        x_control, y_pos, CONTROL_WIDTH, CONTROL_HEIGHT, parent, (HMENU)IDC_FOLLOW_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Follow Strength slider");
        return false;
    }
    SettingsUI_AddTooltip(control, "Controls how quickly the cursor follows the target (0.05-1.0)");
    
    
    y_pos += CONTROL_SPACING;
    
    // Ease Type
    control = CreateWindow("STATIC", "Ease Type:", WS_CHILD | WS_VISIBLE,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Ease Type label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    HWND ease_combo = CreateWindow("COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        x_control, y_pos, CONTROL_WIDTH, 120, parent, (HMENU)IDC_EASE_COMBO,
        GetModuleHandle(NULL), NULL);
    if (!ease_combo) {
        LOG_ERROR("Failed to create Ease Type combo");
        return false;
    }
    SettingsUI_ApplyFont(ease_combo);
    SettingsUI_AddTooltip(ease_combo, "Animation curve for cursor movement");
    
    // Populate ease combo
    ComboBox_AddString(ease_combo, "Linear");
    ComboBox_AddString(ease_combo, "Ease In");
    ComboBox_AddString(ease_combo, "Ease Out");
    ComboBox_AddString(ease_combo, "Ease In-Out");
    
    y_pos += CONTROL_SPACING;
    
    // Delay Start
    control = CreateWindow("STATIC", "Delay Start (ms):", WS_CHILD | WS_VISIBLE,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Delay Start label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_TOOLTIPS | TBS_ENABLESELRANGE,
        x_control, y_pos, CONTROL_WIDTH, CONTROL_HEIGHT, parent, (HMENU)IDC_DELAY_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Delay Start slider");
        return false;
    }
    SettingsUI_AddTooltip(control, "Delay before stabilization starts (0-500ms)");
    
    
    y_pos += CONTROL_SPACING;
    
    // Dual Mode
    control = CreateWindow("BUTTON", "Enable Dual Mode (velocity adaptive)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x_label, y_pos, LABEL_WIDTH + CONTROL_WIDTH, CONTROL_HEIGHT, parent, (HMENU)IDC_DUAL_CHECK,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Dual Mode checkbox");
        return false;
    }
    SettingsUI_ApplyFont(control);
    SettingsUI_AddTooltip(control, "Adapt stabilization based on mouse movement velocity");
    
    LOG_DEBUG("Basic tab controls created successfully");
    return true;
}

bool SettingsUI_CreateVisualTab(HWND hwnd) {
    LOG_DEBUG("Creating Visual tab controls");
    
    // Visual controls will be hidden initially
    int y_pos = 55;
    int x_label = 30;
    int x_control = x_label + LABEL_WIDTH + 10;
    HWND parent = hwnd;
    HWND control;
    
    // Pointer Type
    control = CreateWindow("STATIC", "Pointer Type:", WS_CHILD,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Pointer Type label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    HWND pointer_combo = CreateWindow("COMBOBOX", NULL,
        WS_CHILD | CBS_DROPDOWNLIST,
        x_control, y_pos, CONTROL_WIDTH, 120, parent, (HMENU)IDC_POINTER_TYPE_COMBO,
        GetModuleHandle(NULL), NULL);
    if (!pointer_combo) {
        LOG_ERROR("Failed to create Pointer Type combo");
        return false;
    }
    SettingsUI_ApplyFont(pointer_combo);
    SettingsUI_AddTooltip(pointer_combo, "Choose target pointer appearance");
    
    // Populate pointer type combo
    ComboBox_AddString(pointer_combo, "Circle");
    ComboBox_AddString(pointer_combo, "Cross");
    
    y_pos += CONTROL_SPACING;
    
    // Target Color
    control = CreateWindow("STATIC", "Target Color:", WS_CHILD,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Color label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    control = CreateWindow("BUTTON", "Choose Color...", WS_CHILD | BS_PUSHBUTTON,
        x_control, y_pos, 120, CONTROL_HEIGHT, parent, (HMENU)IDC_TARGET_COLOR_BUTTON,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Color button");
        return false;
    }
    SettingsUI_ApplyFont(control);
    SettingsUI_AddTooltip(control, "Click to choose target pointer color");
    
    y_pos += CONTROL_SPACING;
    
    // Target Size
    control = CreateWindow("STATIC", "Target Size:", WS_CHILD,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Size label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | TBS_HORZ | TBS_TOOLTIPS | TBS_ENABLESELRANGE,
        x_control, y_pos, CONTROL_WIDTH, CONTROL_HEIGHT, parent, (HMENU)IDC_TARGET_SIZE_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Size slider");
        return false;
    }
    SettingsUI_AddTooltip(control, "Size of the target pointer (3-20 pixels)");
    
    
    y_pos += CONTROL_SPACING;
    
    // Target Alpha
    control = CreateWindow("STATIC", "Target Alpha:", WS_CHILD,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Alpha label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    control = CreateWindow(TRACKBAR_CLASS, NULL,
        WS_CHILD | TBS_HORZ | TBS_TOOLTIPS | TBS_ENABLESELRANGE,
        x_control, y_pos, CONTROL_WIDTH, CONTROL_HEIGHT, parent, (HMENU)IDC_TARGET_ALPHA_SLIDER,
        GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Target Alpha slider");
        return false;
    }
    SettingsUI_AddTooltip(control, "Transparency of target pointer (50-255)");
    
    LOG_DEBUG("Visual tab controls created successfully");
    return true;
}

bool SettingsUI_CreateDebugTab(HWND hwnd) {
    LOG_DEBUG("Creating Debug tab controls");
    
    int y_pos = 55;
    int x_label = 30;
    int x_control = x_label + LABEL_WIDTH + 10;
    HWND parent = hwnd;
    HWND control;
    
    // Log Level
    control = CreateWindow("STATIC", "Log Level:", WS_CHILD,
        x_label, y_pos + 5, LABEL_WIDTH, CONTROL_HEIGHT, parent, NULL, GetModuleHandle(NULL), NULL);
    if (!control) {
        LOG_ERROR("Failed to create Log Level label");
        return false;
    }
    SettingsUI_ApplyFont(control);
    
    HWND log_combo = CreateWindow("COMBOBOX", NULL,
        WS_CHILD | CBS_DROPDOWNLIST,
        x_control, y_pos, CONTROL_WIDTH, 120, parent, (HMENU)IDC_LOG_LEVEL_COMBO,
        GetModuleHandle(NULL), NULL);
    if (!log_combo) {
        LOG_ERROR("Failed to create Log Level combo");
        return false;
    }
    SettingsUI_ApplyFont(log_combo);
    SettingsUI_AddTooltip(log_combo, "Set logging verbosity level");
    
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
    if (tab == TAB_BASIC && ((id >= IDC_FOLLOW_SLIDER && id <= IDC_DUAL_CHECK) || id == IDC_ENABLE_CHECK)) {
        should_show = true;
    } else if (tab == TAB_VISUAL && ((id >= IDC_TARGET_COLOR_BUTTON && id <= IDC_TARGET_ALPHA_EDIT) || id == IDC_POINTER_TYPE_COMBO)) {
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
            
        case WM_DESTROY:
            if (g_ui_font && g_ui_font != GetStockObject(DEFAULT_GUI_FONT)) {
                DeleteObject(g_ui_font);
                g_ui_font = NULL;
            }
            if (g_tooltip) {
                DestroyWindow(g_tooltip);
                g_tooltip = NULL;
            }
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
            int id = LOWORD(wParam);
            
            // Handle color picker button
            if (id == IDC_TARGET_COLOR_BUTTON && code == BN_CLICKED) {
                CHOOSECOLOR cc = {0};
                static COLORREF custom_colors[16] = {0};
                
                cc.lStructSize = sizeof(CHOOSECOLOR);
                cc.hwndOwner = g_settings_window;
                cc.lpCustColors = custom_colors;
                cc.rgbResult = g_stabilizer.target_color;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                
                if (ChooseColor(&cc)) {
                    g_stabilizer.target_color = cc.rgbResult;
                    LOG_DEBUG("Target color changed to: RGB(%d,%d,%d)", 
                              GetRValue(g_stabilizer.target_color),
                              GetGValue(g_stabilizer.target_color), 
                              GetBValue(g_stabilizer.target_color));
                    Settings_Save();
                    TargetPointer_UpdateSettings();
                }
                break;
            }
            
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
    
    // Update Enable checkbox
    HWND enable_check = GetDlgItem(g_settings_window, IDC_ENABLE_CHECK);
    if (enable_check) {
        Button_SetCheck(enable_check, g_stabilizer.enabled ? BST_CHECKED : BST_UNCHECKED);
        LOG_DEBUG("Enable checkbox updated: %s", g_stabilizer.enabled ? "checked" : "unchecked");
    } else {
        LOG_WARN("Enable checkbox not found");
    }
    
    // Update Follow Strength
    HWND slider = GetDlgItem(g_settings_window, IDC_FOLLOW_SLIDER);
    if (slider) {
        SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(5, 100));  // 0.05 to 1.0
        SendMessage(slider, TBM_SETPOS, TRUE, (LPARAM)(g_stabilizer.follow_strength * 100));
        LOG_DEBUG("Follow strength slider updated: %.2f", g_stabilizer.follow_strength);
    } else {
        LOG_WARN("Follow strength slider not found");
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
    
    
    // Update Dual Mode
    HWND check = GetDlgItem(g_settings_window, IDC_DUAL_CHECK);
    if (check) {
        Button_SetCheck(check, g_stabilizer.dual_mode ? BST_CHECKED : BST_UNCHECKED);
    }
    
    // Update Pointer Type
    combo = GetDlgItem(g_settings_window, IDC_POINTER_TYPE_COMBO);
    if (combo) {
        ComboBox_SetCurSel(combo, g_stabilizer.pointer_type);
        LOG_DEBUG("Pointer type combo updated: %d", g_stabilizer.pointer_type);
    } else {
        LOG_WARN("Pointer type combo not found");
    }
    
    // Update Target Size
    slider = GetDlgItem(g_settings_window, IDC_TARGET_SIZE_SLIDER);
    if (slider) {
        SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(3, 20));  // 3 to 20 pixels
        SendMessage(slider, TBM_SETPOS, TRUE, g_stabilizer.target_size);
        LOG_DEBUG("Target size slider updated: %d", g_stabilizer.target_size);
    } else {
        LOG_WARN("Target size slider not found");
    }
    
    
    // Update Target Alpha
    slider = GetDlgItem(g_settings_window, IDC_TARGET_ALPHA_SLIDER);
    if (slider) {
        SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(50, 255));  // 50 to 255
        SendMessage(slider, TBM_SETPOS, TRUE, g_stabilizer.target_alpha);
        LOG_DEBUG("Target alpha slider updated: %d", g_stabilizer.target_alpha);
    } else {
        LOG_WARN("Target alpha slider not found");
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
    
    // Apply Enable/Disable setting
    HWND enable_check = GetDlgItem(g_settings_window, IDC_ENABLE_CHECK);
    if (enable_check) {
        bool was_enabled = g_stabilizer.enabled;
        g_stabilizer.enabled = (Button_GetCheck(enable_check) == BST_CHECKED);
        if (was_enabled != g_stabilizer.enabled) {
            LOG_INFO("Mouse stabilizer %s via settings UI", 
                     g_stabilizer.enabled ? "enabled" : "disabled");
            TrayUI_UpdateIcon();
        }
    }
    
    // Apply Follow Strength from slider only
    HWND follow_slider = GetDlgItem(g_settings_window, IDC_FOLLOW_SLIDER);
    if (follow_slider) {
        int slider_value = (int)SendMessage(follow_slider, TBM_GETPOS, 0, 0);
        float new_strength = slider_value / 100.0f;
        if (new_strength >= 0.05f && new_strength <= 1.0f && new_strength != g_stabilizer.follow_strength) {
            g_stabilizer.follow_strength = new_strength;
            LOG_DEBUG("Follow strength changed to: %.2f", g_stabilizer.follow_strength);
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
    
    // Apply Pointer Type
    combo = GetDlgItem(g_settings_window, IDC_POINTER_TYPE_COMBO);
    if (combo) {
        int sel = ComboBox_GetCurSel(combo);
        if (sel >= 0 && sel <= 1) {
            g_stabilizer.pointer_type = (PointerType)sel;
            LOG_DEBUG("Pointer type changed to: %d", g_stabilizer.pointer_type);
            TargetPointer_UpdateSettings();
        }
    }
    
    // Apply Delay from slider only
    HWND delay_slider = GetDlgItem(g_settings_window, IDC_DELAY_SLIDER);
    if (delay_slider) {
        DWORD slider_value = (DWORD)SendMessage(delay_slider, TBM_GETPOS, 0, 0);
        if (slider_value <= 500 && slider_value != g_stabilizer.delay_start_ms) {
            g_stabilizer.delay_start_ms = slider_value;
            LOG_DEBUG("Delay start changed to: %lums", (unsigned long)g_stabilizer.delay_start_ms);
        }
    }
    
    // Apply Dual Mode
    HWND check = GetDlgItem(g_settings_window, IDC_DUAL_CHECK);
    if (check) {
        g_stabilizer.dual_mode = (Button_GetCheck(check) == BST_CHECKED);
    }
    
    // Apply Target Size from slider only
    HWND size_slider = GetDlgItem(g_settings_window, IDC_TARGET_SIZE_SLIDER);
    if (size_slider) {
        int slider_value = (int)SendMessage(size_slider, TBM_GETPOS, 0, 0);
        if (slider_value >= 3 && slider_value <= 20 && slider_value != g_stabilizer.target_size) {
            g_stabilizer.target_size = slider_value;
            LOG_DEBUG("Target size changed to: %d", g_stabilizer.target_size);
            TargetPointer_UpdateSettings();
        }
    }
    
    // Apply Target Alpha from slider only
    HWND alpha_slider = GetDlgItem(g_settings_window, IDC_TARGET_ALPHA_SLIDER);
    if (alpha_slider) {
        int slider_value = (int)SendMessage(alpha_slider, TBM_GETPOS, 0, 0);
        if (slider_value >= 50 && slider_value <= 255 && slider_value != g_stabilizer.target_alpha) {
            g_stabilizer.target_alpha = slider_value;
            LOG_DEBUG("Target alpha changed to: %d", g_stabilizer.target_alpha);
            TargetPointer_UpdateSettings();
        }
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