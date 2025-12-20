#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>
#include "../config/config.h"

// Show settings dialog
// Returns TRUE if settings were changed
BOOL ShowSettingsWindow(HWND parent, Configuration* config, const char* exePath);

// Settings window procedure
LRESULT CALLBACK SettingsWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // SETTINGS_H
