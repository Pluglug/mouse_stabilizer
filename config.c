#include "mouse_stabilizer.h"

// Global log level configuration (default: INFO)
LogLevel g_log_level = LOG_INFO;
static StabilizerProfile g_profiles[MAX_PROFILES];
static int g_profile_count = 0;
static int g_current_profile_index = -1;

static void Settings_GetConfigPath(char* config_path, size_t config_path_size) {
    GetModuleFileName(NULL, config_path, (DWORD)config_path_size);
    char* last_slash = strrchr(config_path, '\\');
    if (last_slash) {
        strcpy_s(last_slash + 1, config_path_size - (size_t)(last_slash + 1 - config_path), "mouse_stabilizer.ini");
    }
}

static void Settings_ValidateCurrent(void) {
    if (g_stabilizer.follow_strength < 0.05f) g_stabilizer.follow_strength = 0.05f;
    if (g_stabilizer.follow_strength > 1.0f) g_stabilizer.follow_strength = 1.0f;
    if (g_stabilizer.min_distance < 0.1f) g_stabilizer.min_distance = 0.1f;
    if (g_stabilizer.min_distance > 5.0f) g_stabilizer.min_distance = 5.0f;
    if (g_stabilizer.ease_type < 0 || g_stabilizer.ease_type > 3) {
        g_stabilizer.ease_type = EASE_OUT;
    }
    if (g_stabilizer.delay_start_ms > 1000) g_stabilizer.delay_start_ms = 1000;
    if (g_stabilizer.target_show_distance < 1.0f) g_stabilizer.target_show_distance = 1.0f;
    if (g_stabilizer.target_show_distance > 50.0f) g_stabilizer.target_show_distance = 50.0f;
    if (g_stabilizer.pointer_type < POINTER_CIRCLE || g_stabilizer.pointer_type > POINTER_CROSS) {
        g_stabilizer.pointer_type = DEFAULT_POINTER_TYPE;
    }
    if (g_stabilizer.target_size < 3) g_stabilizer.target_size = 3;
    if (g_stabilizer.target_size > 20) g_stabilizer.target_size = 20;
    if (g_stabilizer.target_alpha < 50) g_stabilizer.target_alpha = 50;
    if (g_stabilizer.target_alpha > 255) g_stabilizer.target_alpha = 255;
    for (int i = 0; i < 4; i++) {
        if (g_stabilizer.ease_follow_strength[i] < 0.05f) g_stabilizer.ease_follow_strength[i] = 0.05f;
        if (g_stabilizer.ease_follow_strength[i] > 1.0f) g_stabilizer.ease_follow_strength[i] = 1.0f;
        if (g_stabilizer.ease_delay_start_ms[i] > 1000) g_stabilizer.ease_delay_start_ms[i] = 1000;
    }
    if (g_log_level < LOG_ERROR || g_log_level > LOG_TRACE) g_log_level = LOG_INFO;
}

static void Settings_InitializeEaseMemory(float follow_strength, DWORD delay_start_ms) {
    for (int i = 0; i < 4; i++) {
        g_stabilizer.ease_follow_strength[i] = follow_strength;
        g_stabilizer.ease_delay_start_ms[i] = delay_start_ms;
    }
}

static void Settings_LoadEaseMemory(const char* section, const char* config_path) {
    char key[64];
    
    for (int i = 0; i < 4; i++) {
        sprintf_s(key, sizeof(key), "Ease%dFollowStrength", i);
        g_stabilizer.ease_follow_strength[i] =
            (float)GetPrivateProfileInt(section, key, (int)(g_stabilizer.ease_follow_strength[i] * 100), config_path) / 100.0f;
        
        sprintf_s(key, sizeof(key), "Ease%dDelayStartMs", i);
        g_stabilizer.ease_delay_start_ms[i] =
            (DWORD)GetPrivateProfileInt(section, key, (int)g_stabilizer.ease_delay_start_ms[i], config_path);
    }
}

static void Settings_WriteEaseMemory(const char* section, const char* config_path, const float follow_values[4], const unsigned long delay_values[4]) {
    char key[64];
    char buffer[32];
    
    for (int i = 0; i < 4; i++) {
        sprintf_s(key, sizeof(key), "Ease%dFollowStrength", i);
        sprintf_s(buffer, sizeof(buffer), "%d", (int)(follow_values[i] * 100));
        WritePrivateProfileString(section, key, buffer, config_path);
        
        sprintf_s(key, sizeof(key), "Ease%dDelayStartMs", i);
        sprintf_s(buffer, sizeof(buffer), "%lu", delay_values[i]);
        WritePrivateProfileString(section, key, buffer, config_path);
    }
}

static void Settings_ApplyStoredEaseValues(void) {
    int ease = (int)g_stabilizer.ease_type;
    if (ease < 0 || ease > 3) {
        ease = EASE_OUT;
        g_stabilizer.ease_type = EASE_OUT;
    }
    
    g_stabilizer.follow_strength = g_stabilizer.ease_follow_strength[ease];
    g_stabilizer.delay_start_ms = g_stabilizer.ease_delay_start_ms[ease];
    Settings_ValidateCurrent();
}

static void Settings_CopyCurrentToProfile(StabilizerProfile* profile, const char* name) {
    if (!profile) return;
    Settings_RememberCurrentEaseValues();
    if (name && name[0] != '\0') {
        strcpy_s(profile->name, sizeof(profile->name), name);
    }
    profile->follow_strength = g_stabilizer.follow_strength;
    for (int i = 0; i < 4; i++) {
        profile->ease_follow_strength[i] = g_stabilizer.ease_follow_strength[i];
        profile->ease_delay_start_ms[i] = (unsigned long)g_stabilizer.ease_delay_start_ms[i];
    }
    profile->min_distance = g_stabilizer.min_distance;
    profile->ease_type = (int)g_stabilizer.ease_type;
    profile->dual_mode = g_stabilizer.dual_mode;
    profile->enabled = g_stabilizer.enabled;
    profile->delay_start_ms = (unsigned long)g_stabilizer.delay_start_ms;
    profile->target_show_distance = g_stabilizer.target_show_distance;
    profile->pointer_type = (int)g_stabilizer.pointer_type;
    profile->target_size = g_stabilizer.target_size;
    profile->target_alpha = g_stabilizer.target_alpha;
    profile->target_color = (unsigned long)g_stabilizer.target_color;
    profile->target_always_visible = g_stabilizer.target_always_visible;
    profile->exclude_from_capture = g_stabilizer.exclude_from_capture;
    profile->capture_compatibility_mode = g_stabilizer.capture_compatibility_mode;
}

static void Settings_CopyProfileToCurrent(const StabilizerProfile* profile) {
    if (!profile) return;
    g_stabilizer.follow_strength = profile->follow_strength;
    for (int i = 0; i < 4; i++) {
        g_stabilizer.ease_follow_strength[i] = profile->ease_follow_strength[i];
        g_stabilizer.ease_delay_start_ms[i] = (DWORD)profile->ease_delay_start_ms[i];
    }
    g_stabilizer.min_distance = profile->min_distance;
    g_stabilizer.ease_type = (EaseType)profile->ease_type;
    g_stabilizer.dual_mode = profile->dual_mode;
    g_stabilizer.enabled = profile->enabled;
    g_stabilizer.delay_start_ms = (DWORD)profile->delay_start_ms;
    g_stabilizer.target_show_distance = profile->target_show_distance;
    g_stabilizer.pointer_type = (PointerType)profile->pointer_type;
    g_stabilizer.target_size = profile->target_size;
    g_stabilizer.target_alpha = profile->target_alpha;
    g_stabilizer.target_color = (COLORREF)profile->target_color;
    g_stabilizer.target_always_visible = profile->target_always_visible;
    g_stabilizer.exclude_from_capture = profile->exclude_from_capture;
    g_stabilizer.capture_compatibility_mode = profile->capture_compatibility_mode;
    Settings_ValidateCurrent();
    g_stabilizer.ease_follow_strength[(int)g_stabilizer.ease_type] = g_stabilizer.follow_strength;
    g_stabilizer.ease_delay_start_ms[(int)g_stabilizer.ease_type] = g_stabilizer.delay_start_ms;
}

static bool Settings_IsBlankName(const char* name) {
    if (!name) return true;
    while (*name) {
        if (*name != ' ' && *name != '\t' && *name != '\r' && *name != '\n') {
            return false;
        }
        name++;
    }
    return true;
}

static int Settings_FindProfileByName(const char* name) {
    if (Settings_IsBlankName(name)) return -1;
    for (int i = 0; i < g_profile_count; i++) {
        if (lstrcmpiA(g_profiles[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void Settings_LoadProfiles(const char* config_path) {
    char current_name[PROFILE_NAME_MAX] = {0};
    g_profile_count = GetPrivateProfileInt("Profiles", "Count", 0, config_path);
    if (g_profile_count < 0) g_profile_count = 0;
    if (g_profile_count > MAX_PROFILES) g_profile_count = MAX_PROFILES;
    
    GetPrivateProfileString("Profiles", "Current", "", current_name, sizeof(current_name), config_path);
    
    for (int i = 0; i < g_profile_count; i++) {
        char section[32];
        sprintf_s(section, sizeof(section), "Profile%d", i);
        GetPrivateProfileString(section, "Name", "", g_profiles[i].name, sizeof(g_profiles[i].name), config_path);
        if (Settings_IsBlankName(g_profiles[i].name)) {
            sprintf_s(g_profiles[i].name, sizeof(g_profiles[i].name), "Profile %d", i + 1);
        }
        
        g_profiles[i].follow_strength = (float)GetPrivateProfileInt(section, "FollowStrength", (int)(DEFAULT_FOLLOW_STRENGTH * 100), config_path) / 100.0f;
        for (int ease = 0; ease < 4; ease++) {
            char key[64];
            sprintf_s(key, sizeof(key), "Ease%dFollowStrength", ease);
            g_profiles[i].ease_follow_strength[ease] =
                (float)GetPrivateProfileInt(section, key, (int)(g_profiles[i].follow_strength * 100), config_path) / 100.0f;
        }
        g_profiles[i].min_distance = (float)GetPrivateProfileInt(section, "MinDistance", (int)(DEFAULT_MIN_DISTANCE * 10), config_path) / 10.0f;
        g_profiles[i].ease_type = GetPrivateProfileInt(section, "EaseType", EASE_OUT, config_path);
        g_profiles[i].dual_mode = GetPrivateProfileInt(section, "DualMode", 1, config_path) != 0;
        g_profiles[i].enabled = GetPrivateProfileInt(section, "Enabled", 1, config_path) != 0;
        g_profiles[i].delay_start_ms = (unsigned long)GetPrivateProfileInt(section, "DelayStartMs", DEFAULT_DELAY_START_MS, config_path);
        for (int ease = 0; ease < 4; ease++) {
            char key[64];
            sprintf_s(key, sizeof(key), "Ease%dDelayStartMs", ease);
            g_profiles[i].ease_delay_start_ms[ease] =
                (unsigned long)GetPrivateProfileInt(section, key, (int)g_profiles[i].delay_start_ms, config_path);
        }
        g_profiles[i].target_show_distance = (float)GetPrivateProfileInt(section, "TargetShowDistance", (int)(DEFAULT_TARGET_SHOW_DISTANCE * 10), config_path) / 10.0f;
        g_profiles[i].pointer_type = GetPrivateProfileInt(section, "PointerType", DEFAULT_POINTER_TYPE, config_path);
        g_profiles[i].target_size = GetPrivateProfileInt(section, "TargetSize", DEFAULT_TARGET_SIZE, config_path);
        g_profiles[i].target_alpha = GetPrivateProfileInt(section, "TargetAlpha", DEFAULT_TARGET_ALPHA, config_path);
        g_profiles[i].target_color = (unsigned long)GetPrivateProfileInt(section, "TargetColor", RGB(255, 100, 100), config_path);
        g_profiles[i].target_always_visible = GetPrivateProfileInt(section, "TargetAlwaysVisible", DEFAULT_TARGET_ALWAYS_VISIBLE ? 1 : 0, config_path) != 0;
        g_profiles[i].exclude_from_capture = GetPrivateProfileInt(section, "ExcludeFromCapture", DEFAULT_EXCLUDE_FROM_CAPTURE ? 1 : 0, config_path) != 0;
        g_profiles[i].capture_compatibility_mode = GetPrivateProfileInt(section, "CaptureCompatibilityMode", DEFAULT_CAPTURE_COMPATIBILITY_MODE ? 1 : 0, config_path) != 0;
    }
    
    g_current_profile_index = Settings_FindProfileByName(current_name);
}

static void Settings_SaveProfiles(const char* config_path) {
    char buffer[64];
    sprintf_s(buffer, sizeof(buffer), "%d", g_profile_count);
    WritePrivateProfileString("Profiles", "Count", buffer, config_path);
    WritePrivateProfileString("Profiles", "Current", Settings_GetCurrentProfileName(), config_path);
    
    for (int i = 0; i < MAX_PROFILES; i++) {
        char section[32];
        sprintf_s(section, sizeof(section), "Profile%d", i);
        WritePrivateProfileString(section, NULL, NULL, config_path);
    }
    
    for (int i = 0; i < g_profile_count; i++) {
        char section[32];
        sprintf_s(section, sizeof(section), "Profile%d", i);
        WritePrivateProfileString(section, "Name", g_profiles[i].name, config_path);
        
        sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_profiles[i].follow_strength * 100));
        WritePrivateProfileString(section, "FollowStrength", buffer, config_path);
        Settings_WriteEaseMemory(section, config_path, g_profiles[i].ease_follow_strength, g_profiles[i].ease_delay_start_ms);
        sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_profiles[i].min_distance * 10));
        WritePrivateProfileString(section, "MinDistance", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].ease_type);
        WritePrivateProfileString(section, "EaseType", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].dual_mode ? 1 : 0);
        WritePrivateProfileString(section, "DualMode", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].enabled ? 1 : 0);
        WritePrivateProfileString(section, "Enabled", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%lu", g_profiles[i].delay_start_ms);
        WritePrivateProfileString(section, "DelayStartMs", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_profiles[i].target_show_distance * 10));
        WritePrivateProfileString(section, "TargetShowDistance", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].pointer_type);
        WritePrivateProfileString(section, "PointerType", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].target_size);
        WritePrivateProfileString(section, "TargetSize", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].target_alpha);
        WritePrivateProfileString(section, "TargetAlpha", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%lu", g_profiles[i].target_color);
        WritePrivateProfileString(section, "TargetColor", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].target_always_visible ? 1 : 0);
        WritePrivateProfileString(section, "TargetAlwaysVisible", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].exclude_from_capture ? 1 : 0);
        WritePrivateProfileString(section, "ExcludeFromCapture", buffer, config_path);
        sprintf_s(buffer, sizeof(buffer), "%d", g_profiles[i].capture_compatibility_mode ? 1 : 0);
        WritePrivateProfileString(section, "CaptureCompatibilityMode", buffer, config_path);
    }
}

void Settings_WriteLog(const char* format, ...) {
    static FILE* log_file = NULL;
    static bool first_call = true;
    
    if (first_call) {
        char log_path[MAX_PATH];
        GetModuleFileName(NULL, log_path, MAX_PATH);
        char* last_slash = strrchr(log_path, '\\');
        if (last_slash) {
            strcpy_s(last_slash + 1, MAX_PATH - (last_slash + 1 - log_path), "mouse_stabilizer.log");
        }
        
        if (fopen_s(&log_file, log_path, "a") != 0) {
            log_file = NULL;
            // Cannot log here since logging system is not yet initialized
        }
        first_call = false;
        
        if (log_file) {
            time_t now = time(NULL);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now);
            
            fprintf(log_file, "\n=== Mouse Stabilizer Started: %04d-%02d-%02d %02d:%02d:%02d ===\n",
                    timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            fflush(log_file);
        }
    }
    
    if (!log_file) return;
    
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    
    fprintf(log_file, "[%02d:%02d:%02d] ", 
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

void Settings_Load(void) {
    char config_path[MAX_PATH];
    Settings_GetConfigPath(config_path, sizeof(config_path));
    
    g_stabilizer.follow_strength = (float)GetPrivateProfileInt("Settings", "FollowStrength", 
                                                               (int)(DEFAULT_FOLLOW_STRENGTH * 100), 
                                                               config_path) / 100.0f;
    
    g_stabilizer.min_distance = (float)GetPrivateProfileInt("Settings", "MinDistance", 
                                                            (int)(DEFAULT_MIN_DISTANCE * 10), 
                                                            config_path) / 10.0f;
    
    g_stabilizer.ease_type = (EaseType)GetPrivateProfileInt("Settings", "EaseType", 
                                                            EASE_OUT, config_path);
    
    g_stabilizer.dual_mode = GetPrivateProfileInt("Settings", "DualMode", 1, config_path) != 0;
    g_stabilizer.enabled = GetPrivateProfileInt("Settings", "Enabled", 1, config_path) != 0;
    
    g_stabilizer.delay_start_ms = GetPrivateProfileInt("Settings", "DelayStartMs", 
                                                       DEFAULT_DELAY_START_MS, config_path);
    g_stabilizer.target_show_distance = (float)GetPrivateProfileInt("Settings", "TargetShowDistance", 
                                                                    (int)(DEFAULT_TARGET_SHOW_DISTANCE * 10), 
                                                                    config_path) / 10.0f;
    g_stabilizer.pointer_type = (PointerType)GetPrivateProfileInt("Settings", "PointerType", 
                                                                  DEFAULT_POINTER_TYPE, config_path);
    g_stabilizer.target_size = GetPrivateProfileInt("Settings", "TargetSize", 
                                                    DEFAULT_TARGET_SIZE, config_path);
    g_stabilizer.target_alpha = GetPrivateProfileInt("Settings", "TargetAlpha", 
                                                     DEFAULT_TARGET_ALPHA, config_path);
    g_stabilizer.target_color = GetPrivateProfileInt("Settings", "TargetColor", 
                                                     RGB(255, 100, 100), config_path);
    
    // Load log level setting
    g_log_level = (LogLevel)GetPrivateProfileInt("Settings", "LogLevel", 
                                                 LOG_DEBUG, config_path);  // Default to DEBUG for now
    
    // Load capture exclusion settings
    g_stabilizer.exclude_from_capture = GetPrivateProfileInt("Settings", "ExcludeFromCapture", 
                                                             DEFAULT_EXCLUDE_FROM_CAPTURE ? 1 : 0, config_path) != 0;
    g_stabilizer.capture_compatibility_mode = GetPrivateProfileInt("Settings", "CaptureCompatibilityMode", 
                                                                   DEFAULT_CAPTURE_COMPATIBILITY_MODE ? 1 : 0, config_path) != 0;
    
    // Load target visibility setting
    g_stabilizer.target_always_visible = GetPrivateProfileInt("Settings", "TargetAlwaysVisible", 
                                                              DEFAULT_TARGET_ALWAYS_VISIBLE ? 1 : 0, config_path) != 0;
    
    Settings_ValidateCurrent();
    Settings_InitializeEaseMemory(g_stabilizer.follow_strength, g_stabilizer.delay_start_ms);
    Settings_LoadEaseMemory("Settings", config_path);
    Settings_ApplyStoredEaseValues();
    Settings_LoadProfiles(config_path);
    
    Settings_WriteLog("Settings loaded - Follow: %.2f, Ease: %d, Dual: %s, Delay: %dms, TargetDist: %.1f, Enabled: %s, Profiles: %d",
             g_stabilizer.follow_strength, g_stabilizer.ease_type,
             g_stabilizer.dual_mode ? "true" : "false", g_stabilizer.delay_start_ms,
             g_stabilizer.target_show_distance, g_stabilizer.enabled ? "true" : "false", g_profile_count);
}

void Settings_Save(void) {
    char config_path[MAX_PATH];
    Settings_GetConfigPath(config_path, sizeof(config_path));
    
    char buffer[32];
    Settings_RememberCurrentEaseValues();
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.follow_strength * 100));
    WritePrivateProfileString("Settings", "FollowStrength", buffer, config_path);
    Settings_WriteEaseMemory("Settings", config_path, g_stabilizer.ease_follow_strength, g_stabilizer.ease_delay_start_ms);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.min_distance * 10));
    WritePrivateProfileString("Settings", "MinDistance", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_stabilizer.ease_type);
    WritePrivateProfileString("Settings", "EaseType", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.dual_mode ? 1 : 0);
    WritePrivateProfileString("Settings", "DualMode", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.enabled ? 1 : 0);
    WritePrivateProfileString("Settings", "Enabled", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%lu", (unsigned long)g_stabilizer.delay_start_ms);
    WritePrivateProfileString("Settings", "DelayStartMs", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.target_show_distance * 10));
    WritePrivateProfileString("Settings", "TargetShowDistance", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_stabilizer.pointer_type);
    WritePrivateProfileString("Settings", "PointerType", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.target_size);
    WritePrivateProfileString("Settings", "TargetSize", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.target_alpha);
    WritePrivateProfileString("Settings", "TargetAlpha", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_stabilizer.target_color);
    WritePrivateProfileString("Settings", "TargetColor", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_log_level);
    WritePrivateProfileString("Settings", "LogLevel", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.exclude_from_capture ? 1 : 0);
    WritePrivateProfileString("Settings", "ExcludeFromCapture", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.capture_compatibility_mode ? 1 : 0);
    WritePrivateProfileString("Settings", "CaptureCompatibilityMode", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.target_always_visible ? 1 : 0);
    WritePrivateProfileString("Settings", "TargetAlwaysVisible", buffer, config_path);
    
    Settings_SaveProfiles(config_path);
    
    Settings_WriteLog("Settings saved");
}

int Settings_GetProfileCount(void) {
    return g_profile_count;
}

int Settings_GetCurrentProfileIndex(void) {
    return g_current_profile_index;
}

const char* Settings_GetProfileName(int index) {
    if (index < 0 || index >= g_profile_count) {
        return "";
    }
    return g_profiles[index].name;
}

const char* Settings_GetCurrentProfileName(void) {
    if (g_current_profile_index < 0 || g_current_profile_index >= g_profile_count) {
        return "";
    }
    return g_profiles[g_current_profile_index].name;
}

bool Settings_ApplyProfile(int index) {
    if (index < 0 || index >= g_profile_count) {
        return false;
    }
    
    Settings_CopyProfileToCurrent(&g_profiles[index]);
    g_current_profile_index = index;
    Settings_Save();
    LOG_INFO("Applied profile: %s", g_profiles[index].name);
    return true;
}

bool Settings_SaveCurrentAsProfile(const char* name) {
    if (Settings_IsBlankName(name)) {
        return false;
    }
    
    int existing = Settings_FindProfileByName(name);
    if (existing >= 0) {
        Settings_CopyCurrentToProfile(&g_profiles[existing], name);
        g_current_profile_index = existing;
        Settings_Save();
        LOG_INFO("Updated existing profile: %s", g_profiles[existing].name);
        return true;
    }
    
    if (g_profile_count >= MAX_PROFILES) {
        LOG_WARN("Cannot create profile '%s': profile limit reached", name);
        return false;
    }
    
    Settings_CopyCurrentToProfile(&g_profiles[g_profile_count], name);
    g_current_profile_index = g_profile_count;
    g_profile_count++;
    Settings_Save();
    LOG_INFO("Created profile: %s", name);
    return true;
}

bool Settings_UpdateCurrentProfile(void) {
    if (g_current_profile_index < 0 || g_current_profile_index >= g_profile_count) {
        return false;
    }
    
    char name[PROFILE_NAME_MAX];
    strcpy_s(name, sizeof(name), g_profiles[g_current_profile_index].name);
    Settings_CopyCurrentToProfile(&g_profiles[g_current_profile_index], name);
    Settings_Save();
    LOG_INFO("Updated current profile: %s", name);
    return true;
}

bool Settings_RenameCurrentProfile(const char* name) {
    if (g_current_profile_index < 0 || g_current_profile_index >= g_profile_count || Settings_IsBlankName(name)) {
        return false;
    }
    
    int existing = Settings_FindProfileByName(name);
    if (existing >= 0 && existing != g_current_profile_index) {
        return false;
    }
    
    strcpy_s(g_profiles[g_current_profile_index].name, sizeof(g_profiles[g_current_profile_index].name), name);
    Settings_Save();
    LOG_INFO("Renamed current profile: %s", name);
    return true;
}

bool Settings_DeleteCurrentProfile(void) {
    if (g_current_profile_index < 0 || g_current_profile_index >= g_profile_count) {
        return false;
    }
    
    char deleted_name[PROFILE_NAME_MAX];
    strcpy_s(deleted_name, sizeof(deleted_name), g_profiles[g_current_profile_index].name);
    
    for (int i = g_current_profile_index; i < g_profile_count - 1; i++) {
        g_profiles[i] = g_profiles[i + 1];
    }
    g_profile_count--;
    g_current_profile_index = -1;
    Settings_Save();
    LOG_INFO("Deleted profile: %s", deleted_name);
    return true;
}

void Settings_RememberCurrentEaseValues(void) {
    int ease = (int)g_stabilizer.ease_type;
    if (ease < 0 || ease > 3) return;
    
    g_stabilizer.ease_follow_strength[ease] = g_stabilizer.follow_strength;
    g_stabilizer.ease_delay_start_ms[ease] = g_stabilizer.delay_start_ms;
}

bool Settings_SwitchEaseType(int ease_type) {
    if (ease_type < 0 || ease_type > 3) {
        return false;
    }
    
    Settings_RememberCurrentEaseValues();
    g_stabilizer.ease_type = (EaseType)ease_type;
    g_stabilizer.follow_strength = g_stabilizer.ease_follow_strength[ease_type];
    g_stabilizer.delay_start_ms = g_stabilizer.ease_delay_start_ms[ease_type];
    Settings_ValidateCurrent();
    LOG_DEBUG("Switched ease type to %d with follow %.2f and delay %lums",
              ease_type, g_stabilizer.follow_strength, (unsigned long)g_stabilizer.delay_start_ms);
    return true;
}

/**
 * Enhanced logging system with level control
 */

void Settings_WriteLogLevel(LogLevel level, const char* format, ...) {
    // Skip logging if current level is below the message level
    if (level > g_log_level) {
        return;
    }
    
    static FILE* log_file = NULL;
    static bool first_call = true;
    
    if (first_call) {
        char log_path[MAX_PATH];
        GetModuleFileName(NULL, log_path, MAX_PATH);
        char* last_slash = strrchr(log_path, '\\');
        if (last_slash) {
            strcpy_s(last_slash + 1, MAX_PATH - (last_slash + 1 - log_path), "mouse_stabilizer.log");
        }
        
        if (fopen_s(&log_file, log_path, "a") != 0) {
            log_file = NULL;
        }
        first_call = false;
        
        if (log_file) {
            time_t now = time(NULL);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now);
            
            fprintf(log_file, "\n=== Mouse Stabilizer Started: %04d-%02d-%02d %02d:%02d:%02d ===\n",
                    timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            fflush(log_file);
        }
    }
    
    if (!log_file) return;
    
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    
    // Add log level prefix to timestamp
    const char* level_str = Settings_GetLogLevelName(level);
    fprintf(log_file, "[%02d:%02d:%02d %s] ", 
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, level_str);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

void Settings_SetLogLevel(LogLevel level) {
    g_log_level = level;
    LOG_INFO("Log level changed to: %s", Settings_GetLogLevelName(level));
}

LogLevel Settings_GetLogLevel(void) {
    return g_log_level;
}

const char* Settings_GetLogLevelName(LogLevel level) {
    switch (level) {
        case LOG_ERROR: return "ERROR";
        case LOG_WARN:  return "WARN ";
        case LOG_INFO:  return "INFO ";
        case LOG_DEBUG: return "DEBUG";
        case LOG_TRACE: return "TRACE";
        default:        return "UNKN ";
    }
}
