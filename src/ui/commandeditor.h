#ifndef COMMANDEDITOR_H
#define COMMANDEDITOR_H

#include <windows.h>
#include "../config/config.h"

// Show command editor dialog
// If command is NULL, creates a new command
// Returns TRUE if command was saved
BOOL ShowCommandEditor(HWND parent, Command* command, BOOL isNew);

// Command editor window procedure
LRESULT CALLBACK CommandEditorProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // COMMANDEDITOR_H
