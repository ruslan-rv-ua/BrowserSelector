#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include "../config/config.h"

typedef struct {
    HWND hwnd;
    HWND listBox;
    HWND registerBtn;
    HWND settingsBtn;
    HWND countdownLabel;  // Label for countdown display
    Configuration* config;
    char* url;
    char exePath[MAX_PATH];
    HINSTANCE hInstance;
    
    // Timer fields
    UINT_PTR timerId;
    int remainingSeconds;
    BOOL timerActive;
} MainWindow;

// Create and show main window
HWND CreateMainWindow(HINSTANCE hInstance, Configuration* config, const char* url, const char* exePath);

// Window procedure
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Refresh list box with current commands
void RefreshCommandList(MainWindow* mainWin);

#endif // MAINWINDOW_H
