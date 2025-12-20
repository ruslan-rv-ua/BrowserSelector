#include "commandeditor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commdlg.h>

#define ID_NAME_EDIT 2001
#define ID_COMMAND_EDIT 2002
#define ID_ARGS_EDIT 2003
#define ID_BROWSE_BTN 2004
#define ID_OK_BTN 2005
#define ID_CANCEL_BTN 2006

typedef struct {
    Command* command;
    BOOL isNew;
    BOOL saved;
} CommandEditorData;

static const char* CMD_EDITOR_CLASS = "CommandEditorClass";

static void BrowseForExecutable(HWND hwnd, HWND editControl) {
    OPENFILENAMEA ofn;
    char filename[MAX_PATH] = "";
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Select Browser Executable";
    
    if (GetOpenFileNameA(&ofn)) {
        SetWindowTextA(editControl, filename);
    }
}

LRESULT CALLBACK CommandEditorProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    CommandEditorData* data = (CommandEditorData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            data = (CommandEditorData*)cs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
            
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            
            // Name label and edit
            HWND label = CreateWindowExA(0, "STATIC", "Name:",
                WS_CHILD | WS_VISIBLE,
                10, 15, 60, 20, hwnd, NULL, cs->hInstance, NULL);
            SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND nameEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", data->command->name,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                80, 12, 280, 24, hwnd, (HMENU)ID_NAME_EDIT, cs->hInstance, NULL);
            SendMessage(nameEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(nameEdit, EM_SETLIMITTEXT, MAX_NAME_LENGTH - 1, 0);
            
            // Command label and edit
            label = CreateWindowExA(0, "STATIC", "Command:",
                WS_CHILD | WS_VISIBLE,
                10, 50, 60, 20, hwnd, NULL, cs->hInstance, NULL);
            SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND cmdEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", data->command->command,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                80, 47, 220, 24, hwnd, (HMENU)ID_COMMAND_EDIT, cs->hInstance, NULL);
            SendMessage(cmdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(cmdEdit, EM_SETLIMITTEXT, MAX_PATH_LENGTH - 1, 0);
            
            // Browse button
            HWND browseBtn = CreateWindowExA(0, "BUTTON", "...",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                305, 47, 30, 24, hwnd, (HMENU)ID_BROWSE_BTN, cs->hInstance, NULL);
            SendMessage(browseBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Arguments label and edit
            label = CreateWindowExA(0, "STATIC", "Arguments:",
                WS_CHILD | WS_VISIBLE,
                10, 85, 65, 20, hwnd, NULL, cs->hInstance, NULL);
            SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND argsEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", data->command->arguments,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                80, 82, 255, 24, hwnd, (HMENU)ID_ARGS_EDIT, cs->hInstance, NULL);
            SendMessage(argsEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(argsEdit, EM_SETLIMITTEXT, MAX_PATH_LENGTH - 1, 0);
            
            // Help text
            label = CreateWindowExA(0, "STATIC", "Use {url} as placeholder for the URL",
                WS_CHILD | WS_VISIBLE,
                80, 110, 250, 20, hwnd, NULL, cs->hInstance, NULL);
            SendMessage(label, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // OK and Cancel buttons
            HWND okBtn = CreateWindowExA(0, "BUTTON", "OK",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                170, 145, 80, 28, hwnd, (HMENU)ID_OK_BTN, cs->hInstance, NULL);
            SendMessage(okBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND cancelBtn = CreateWindowExA(0, "BUTTON", "Cancel",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                255, 145, 80, 28, hwnd, (HMENU)ID_CANCEL_BTN, cs->hInstance, NULL);
            SendMessage(cancelBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Focus on name field
            SetFocus(nameEdit);
            
            return 0;
        }
        
        case WM_COMMAND: {
            if (!data) break;
            
            switch (LOWORD(wParam)) {
                case ID_BROWSE_BTN: {
                    HWND cmdEdit = GetDlgItem(hwnd, ID_COMMAND_EDIT);
                    BrowseForExecutable(hwnd, cmdEdit);
                    return 0;
                }
                
                case ID_OK_BTN: {
                    // Validate and save
                    char name[MAX_NAME_LENGTH];
                    char cmd[MAX_PATH_LENGTH];
                    char args[MAX_PATH_LENGTH];
                    
                    GetDlgItemTextA(hwnd, ID_NAME_EDIT, name, MAX_NAME_LENGTH);
                    GetDlgItemTextA(hwnd, ID_COMMAND_EDIT, cmd, MAX_PATH_LENGTH);
                    GetDlgItemTextA(hwnd, ID_ARGS_EDIT, args, MAX_PATH_LENGTH);
                    
                    // Trim whitespace
                    char* p = name;
                    while (*p == ' ') p++;
                    if (*p == '\0') {
                        MessageBoxA(hwnd, "Name cannot be empty", "Validation Error", MB_OK | MB_ICONWARNING);
                        SetFocus(GetDlgItem(hwnd, ID_NAME_EDIT));
                        return 0;
                    }
                    
                    p = cmd;
                    while (*p == ' ') p++;
                    if (*p == '\0') {
                        MessageBoxA(hwnd, "Command cannot be empty", "Validation Error", MB_OK | MB_ICONWARNING);
                        SetFocus(GetDlgItem(hwnd, ID_COMMAND_EDIT));
                        return 0;
                    }
                    
                    // Save data
                    snprintf(data->command->name, MAX_NAME_LENGTH, "%s", name);
                    snprintf(data->command->command, MAX_PATH_LENGTH, "%s", cmd);
                    snprintf(data->command->arguments, MAX_PATH_LENGTH, "%s", args);
                    
                    data->saved = TRUE;
                    DestroyWindow(hwnd);
                    return 0;
                }
                
                case ID_CANCEL_BTN:
                    data->saved = FALSE;
                    DestroyWindow(hwnd);
                    return 0;
            }
            break;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                if (data) data->saved = FALSE;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        
        case WM_CLOSE: {
            if (data) data->saved = FALSE;
            DestroyWindow(hwnd);
            return 0;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL ShowCommandEditor(HWND parent, Command* command, BOOL isNew) {
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    
    // Register window class
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = CommandEditorProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CMD_EDITOR_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassExA(&wc);
    
    // Prepare data
    CommandEditorData data;
    data.command = command;
    data.isNew = isNew;
    data.saved = FALSE;
    
    // Calculate position
    RECT parentRect;
    GetWindowRect(parent, &parentRect);
    int winWidth = 360;
    int winHeight = 220;
    int winX = parentRect.left + (parentRect.right - parentRect.left - winWidth) / 2;
    int winY = parentRect.top + (parentRect.bottom - parentRect.top - winHeight) / 2;
    
    // Create window
    const char* title = isNew ? "Add Browser" : "Edit Browser";
    HWND hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        CMD_EDITOR_CLASS,
        title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        winX, winY, winWidth, winHeight,
        parent, NULL, hInstance, &data
    );
    
    if (!hwnd) return FALSE;
    
    // Disable parent window
    EnableWindow(parent, FALSE);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // Modal message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // Re-enable parent
    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);
    
    return data.saved;
}
