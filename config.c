#include "mouse_stabilizer.h"

void WriteLog(const char* format, ...) {
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

void LoadSettings(void) {
    char config_path[MAX_PATH];
    GetModuleFileName(NULL, config_path, MAX_PATH);
    char* last_slash = strrchr(config_path, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, "mouse_stabilizer.ini");
    }
    
    g_stabilizer.smoothing_strength = (float)GetPrivateProfileInt("Settings", "SmoothingStrength", 
                                                                  (int)(DEFAULT_SMOOTHING_STRENGTH * 100), 
                                                                  config_path) / 100.0f;
    
    g_stabilizer.threshold = (float)GetPrivateProfileInt("Settings", "Threshold", 
                                                         (int)(DEFAULT_THRESHOLD * 10), 
                                                         config_path) / 10.0f;
    
    g_stabilizer.filter_type = (FilterType)GetPrivateProfileInt("Settings", "FilterType", 
                                                                FILTER_EXPONENTIAL, config_path);
    
    g_stabilizer.enabled = GetPrivateProfileInt("Settings", "Enabled", 1, config_path) != 0;
    
    if (g_stabilizer.smoothing_strength < 0.1f) g_stabilizer.smoothing_strength = 0.1f;
    if (g_stabilizer.smoothing_strength > 1.0f) g_stabilizer.smoothing_strength = 1.0f;
    if (g_stabilizer.threshold < 1.0f) g_stabilizer.threshold = 1.0f;
    if (g_stabilizer.threshold > 20.0f) g_stabilizer.threshold = 20.0f;
    if (g_stabilizer.filter_type < 0 || g_stabilizer.filter_type > 2) {
        g_stabilizer.filter_type = FILTER_EXPONENTIAL;
    }
    
    WriteLog("Settings loaded - Smoothing: %.2f, Threshold: %.1f, Filter: %d, Enabled: %s",
             g_stabilizer.smoothing_strength, g_stabilizer.threshold, g_stabilizer.filter_type,
             g_stabilizer.enabled ? "true" : "false");
}

void SaveSettings(void) {
    char config_path[MAX_PATH];
    GetModuleFileName(NULL, config_path, MAX_PATH);
    char* last_slash = strrchr(config_path, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, "mouse_stabilizer.ini");
    }
    
    char buffer[32];
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.smoothing_strength * 100));
    WritePrivateProfileString("Settings", "SmoothingStrength", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)(g_stabilizer.threshold * 10));
    WritePrivateProfileString("Settings", "Threshold", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", (int)g_stabilizer.filter_type);
    WritePrivateProfileString("Settings", "FilterType", buffer, config_path);
    
    sprintf_s(buffer, sizeof(buffer), "%d", g_stabilizer.enabled ? 1 : 0);
    WritePrivateProfileString("Settings", "Enabled", buffer, config_path);
    
    WriteLog("Settings saved");
}