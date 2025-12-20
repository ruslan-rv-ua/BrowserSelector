#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>
#include "config/config.h"
#include "ui/mainwindow.h"
#include "registry/registry.h"

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
    if (!LoadConfig(exePath, &config)) {
        MessageBoxA(NULL, "Failed to load configuration.\n\nA default config.json will be created.", 
                   "Warning", MB_OK | MB_ICONWARNING);
        
        // Try to create default config
        char configPath[MAX_PATH];
        GetConfigPath(exePath, configPath);
        if (!CreateDefaultConfig(configPath)) {
            MessageBoxA(NULL, "Failed to create default configuration.", 
                       "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        // Try loading again
        if (!LoadConfig(exePath, &config)) {
            MessageBoxA(NULL, "Failed to load configuration after creating default.", 
                       "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    }
    
    // Check if we have any commands
    if (config.commandCount == 0) {
        MessageBoxA(NULL, 
            "No browsers configured.\n\n"
            "The settings window will open to add browsers.",
            "No Browsers", MB_OK | MB_ICONINFORMATION);
    }
    
    // Create main window
    HWND hwnd = CreateMainWindow(hInstance, &config, arg, exePath);
    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create main window.", "Error", MB_OK | MB_ICONERROR);
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
