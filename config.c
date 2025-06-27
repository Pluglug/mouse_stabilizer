#include "mouse_stabilizer.h"

void Settings_WriteLog(const char* format, ...) {
    static FILE* log_file = NULL;
    static bool first_call = true;
    
    if (first_call) {
        char log_path[MAX_PATH];
        GetModuleFileName(NULL, log_path, MAX_PATH);
        char* last_slash = strrchr(log_path, '\\');
        if (last_slash) {
            strcpy(last_slash + 1, "mouse_stabilizer.log");
        }
        
        fopen_s(&log_file, log_path, "a");
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
    GetModuleFileName(NULL, config_path, MAX_PATH);
    char* last_slash = strrchr(config_path, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, "mouse_stabilizer.ini");
    }
    
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
    g_stabilizer.target_size = GetPrivateProfileInt("Settings", "TargetSize", 
                                                    DEFAULT_TARGET_SIZE, config_path);
    g_stabilizer.target_alpha = GetPrivateProfileInt("Settings", "TargetAlpha", 
                                                     DEFAULT_TARGET_ALPHA, config_path);
    g_stabilizer.target_color = GetPrivateProfileInt("Settings", "TargetColor", 
                                                     RGB(255, 100, 100), config_path);
    
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
    if (g_stabilizer.target_size < 3) g_stabilizer.target_size = 3;
    if (g_stabilizer.target_size > 20) g_stabilizer.target_size = 20;
    if (g_stabilizer.target_alpha < 50) g_stabilizer.target_alpha = 50;
    if (g_stabilizer.target_alpha > 255) g_stabilizer.target_alpha = 255;
    
    Settings_WriteLog("Settings loaded - Follow: %.2f, Ease: %d, Dual: %s, Delay: %dms, TargetDist: %.1f, Enabled: %s",
             g_stabilizer.follow_strength, g_stabilizer.ease_type,
             g_stabilizer.dual_mode ? "true" : "false", g_stabilizer.delay_start_ms,
             g_stabilizer.target_show_distance, g_stabilizer.enabled ? "true" : "false");
}

void Settings_Save(void) {
    char config_path[MAX_PATH];
    GetModuleFileName(NULL, config_path, MAX_PATH);
    char* last_slash = strrchr(config_path, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, "mouse_stabilizer.ini");
    }
    
    char buffer[32];
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.follow_strength * 100));
    WritePrivateProfileString("Settings", "FollowStrength", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.min_distance * 10));
    WritePrivateProfileString("Settings", "MinDistance", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_stabilizer.ease_type);
    WritePrivateProfileString("Settings", "EaseType", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.dual_mode ? 1 : 0);
    WritePrivateProfileString("Settings", "DualMode", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.enabled ? 1 : 0);
    WritePrivateProfileString("Settings", "Enabled", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.delay_start_ms);
    WritePrivateProfileString("Settings", "DelayStartMs", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.target_show_distance * 10));
    WritePrivateProfileString("Settings", "TargetShowDistance", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.target_size);
    WritePrivateProfileString("Settings", "TargetSize", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.target_alpha);
    WritePrivateProfileString("Settings", "TargetAlpha", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_stabilizer.target_color);
    WritePrivateProfileString("Settings", "TargetColor", buffer, config_path);
    
    Settings_WriteLog("Settings saved");
}