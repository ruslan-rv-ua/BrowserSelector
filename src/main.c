#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>
#include "config/config.h"
#include "ui/mainwindow.h"
#include "registry/registry.h"
#include "i18n/i18n.h"

// Parse command line to extract URL
static char* ParseCommandLine(LPSTR lpCmdLine) {
    if (!lpCmdLine || strlen(lpCmdLine) == 0) {
        return NULL;
    }
    
    char* url = lpCmdLine;
    
    // Skip leading spaces
    while (*url == ' ') url++;
    
    if (*url == '\0') {
        return NULL;
    }
    
    // Handle quoted string
    if (*url == '"') {
        url++;
        char* end = strchr(url, '"');
        if (end) {
            *end = '\0';
        }
    } else {
        // Find end of argument (space or end of string)
        char* end = strchr(url, ' ');
        if (end) {
            *end = '\0';
        }
    }
    
    return strlen(url) > 0 ? url : NULL;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;  // Unused
    
    // Initialize i18n (must be first to load localized strings)
    I18n_Init(hInstance);
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Get executable path
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    // Parse command line
    char* arg = ParseCommandLine(lpCmdLine);
    
    // Load configuration
    Configuration config;
    int loadResult = LoadConfig(exePath, &config);
    
    if (loadResult == CONFIG_PARSE_ERROR) {
        // Config file exists but is corrupted - ask user what to do
        int result = MessageBoxW(NULL, 
            I18n_GetStringW(IDS_CONFIG_CORRUPTED_MSG),
            I18n_GetStringW(IDS_ERROR), 
            MB_YESNO | MB_ICONERROR);
        
        if (result == IDYES) {
            // User chose to create new config
            char configPath[MAX_PATH];
            GetConfigPath(exePath, configPath);
            if (!CreateDefaultConfig(configPath)) {
                MessageBoxW(NULL, I18n_GetStringW(IDS_CONFIG_CREATE_FAILED), 
                           I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
                return 1;
            }
            // Try loading again
            loadResult = LoadConfig(exePath, &config);
            if (loadResult != CONFIG_OK) {
                MessageBoxW(NULL, I18n_GetStringW(IDS_CONFIG_LOAD_RETRY_FAIL), 
                           I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
                return 1;
            }
        } else {
            // User chose to exit
            return 1;
        }
    } else if (loadResult != CONFIG_OK) {
        // Other errors (memory, read error, etc.)
        MessageBoxW(NULL, I18n_GetStringW(IDS_CONFIG_LOAD_FAILED_MSG), 
                   I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Check if we have any commands
    if (config.commandCount == 0) {
        MessageBoxW(NULL, 
            I18n_GetStringW(IDS_NO_BROWSERS_MSG),
            I18n_GetStringW(IDS_NO_BROWSERS_TITLE), MB_OK | MB_ICONINFORMATION);
    }
    
    // Create main window
    HWND hwnd = CreateMainWindow(hInstance, &config, arg, exePath);
    if (!hwnd) {
        MessageBoxW(NULL, I18n_GetStringW(IDS_WINDOW_CREATE_FAILED), 
                   I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
