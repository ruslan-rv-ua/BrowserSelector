#include "settings.h"
#include "commandeditor.h"
#include "../config/config.h"
#include "../i18n/i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>

#define ID_COMMAND_LIST 3001
#define ID_ADD_BTN 3002
#define ID_EDIT_BTN 3003
#define ID_DELETE_BTN 3004
#define ID_MOVE_UP_BTN 3005
#define ID_MOVE_DOWN_BTN 3006
#define ID_SET_DEFAULT_BTN 3007
#define ID_CLOSE_BTN 3008
#define ID_WAITTIME_LABEL 3009
#define ID_WAITTIME_EDIT 3010
#define ID_WAITTIME_SPIN 3011

typedef struct {
    Configuration* config;
    const char* exePath;
    HWND listBox;
    HWND waitTimeEdit;
    HWND waitTimeSpin;
    BOOL changed;
} SettingsData;

static const char* SETTINGS_CLASS = "SettingsWindowClass";

static void RefreshSettingsList(SettingsData* data) {
    SendMessage(data->listBox, LB_RESETCONTENT, 0, 0);
    
    for (int i = 0; i < data->config->commandCount; i++) {
        wchar_t itemText[MAX_NAME_LENGTH + 40];
        wchar_t wideName[MAX_NAME_LENGTH];
        
        // Convert name to wide string
        MultiByteToWideChar(CP_ACP, 0, data->config->commands[i].name, -1, wideName, MAX_NAME_LENGTH);
        
        if (i == data->config->settings.defaultCommandIndex) {
            swprintf(itemText, sizeof(itemText)/sizeof(wchar_t), L"%s %s", 
                    wideName, I18n_GetStringW(IDS_DEFAULT_MARKER));
        } else {
            wcscpy(itemText, wideName);
        }
        SendMessageW(data->listBox, LB_ADDSTRING, 0, (LPARAM)itemText);
    }
}

static void SwapCommands(Configuration* config, int idx1, int idx2) {
    Command temp = config->commands[idx1];
    config->commands[idx1] = config->commands[idx2];
    config->commands[idx2] = temp;
    
    // Update default index if needed
    if (config->settings.defaultCommandIndex == idx1) {
        config->settings.defaultCommandIndex = idx2;
    } else if (config->settings.defaultCommandIndex == idx2) {
        config->settings.defaultCommandIndex = idx1;
    }
}

LRESULT CALLBACK SettingsWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsData* data = (SettingsData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            data = (SettingsData*)cs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
            
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            
            // List box for commands
            data->listBox = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"LISTBOX",
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP |
                LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                10, 10, 280, 180,
                hwnd, (HMENU)ID_COMMAND_LIST, cs->hInstance, NULL
            );
            SendMessage(data->listBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // WaitTime controls
            HWND waitTimeLabel = CreateWindowExW(
                0, L"STATIC", I18n_GetStringW(IDS_WAITTIME_LABEL),
                WS_CHILD | WS_VISIBLE,
                10, 200, 150, 20,
                hwnd, (HMENU)ID_WAITTIME_LABEL, cs->hInstance, NULL
            );
            SendMessage(waitTimeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            data->waitTimeEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL,
                170, 200, 50, 20,
                hwnd, (HMENU)ID_WAITTIME_EDIT, cs->hInstance, NULL
            );
            SendMessage(data->waitTimeEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Create spin control
            data->waitTimeSpin = CreateWindowExW(
                0, UPDOWN_CLASSW, NULL,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_NOTHOUSANDS,
                220, 200, 20, 20,
                hwnd, (HMENU)ID_WAITTIME_SPIN, cs->hInstance, NULL
            );
            SendMessage(data->waitTimeSpin, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Set spin control range and buddy
            SendMessage(data->waitTimeSpin, UDM_SETRANGE, 0, MAKELONG(10, 1)); // Min=1, Max=10
            SendMessage(data->waitTimeSpin, UDM_SETBUDDY, (WPARAM)data->waitTimeEdit, 0);
            SendMessage(data->waitTimeSpin, UDM_SETPOS, 0, MAKELONG(data->config->settings.waitTime, 0));
            
            // Buttons
            int btnX = 300;
            int btnWidth = 90;
            int btnHeight = 26;
            int btnSpacing = 32;
            
            HWND btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_ADD_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 10, btnWidth, btnHeight,
                hwnd, (HMENU)ID_ADD_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_EDIT_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 10 + btnSpacing, btnWidth, btnHeight,
                hwnd, (HMENU)ID_EDIT_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_DELETE_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 10 + btnSpacing * 2, btnWidth, btnHeight,
                hwnd, (HMENU)ID_DELETE_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_MOVE_UP_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 10 + btnSpacing * 3, btnWidth, btnHeight,
                hwnd, (HMENU)ID_MOVE_UP_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_MOVE_DOWN_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 10 + btnSpacing * 4, btnWidth, btnHeight,
                hwnd, (HMENU)ID_MOVE_DOWN_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_SET_DEFAULT_CMD_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 10 + btnSpacing * 5, btnWidth, btnHeight,
                hwnd, (HMENU)ID_SET_DEFAULT_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            btn = CreateWindowExW(0, L"BUTTON", I18n_GetStringW(IDS_CLOSE_BTN),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnX, 220 - btnHeight, btnWidth, btnHeight,
                hwnd, (HMENU)ID_CLOSE_BTN, cs->hInstance, NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Fill list
            RefreshSettingsList(data);
            
            if (data->config->commandCount > 0) {
                SendMessage(data->listBox, LB_SETCURSEL, 0, 0);
            }
            
            SetFocus(data->listBox);
            
            return 0;
        }
        
        case WM_COMMAND: {
            if (!data) break;
            
            int sel = (int)SendMessage(data->listBox, LB_GETCURSEL, 0, 0);
            
            // Handle WaitTime edit control
            if (LOWORD(wParam) == ID_WAITTIME_EDIT) {
                if (HIWORD(wParam) == EN_CHANGE) {
                    char buffer[10];
                    GetWindowTextA(data->waitTimeEdit, buffer, sizeof(buffer));
                    
                    int waitTime = atoi(buffer);
                    if (waitTime < 1) {
                        waitTime = 1;
                        snprintf(buffer, sizeof(buffer), "%d", waitTime);
                        SetWindowTextA(data->waitTimeEdit, buffer);
                        SendMessage(data->waitTimeSpin, UDM_SETPOS, 0, MAKELONG(waitTime, 0));
                    } else if (waitTime > 10) {
                        waitTime = 10;
                        snprintf(buffer, sizeof(buffer), "%d", waitTime);
                        SetWindowTextA(data->waitTimeEdit, buffer);
                        SendMessage(data->waitTimeSpin, UDM_SETPOS, 0, MAKELONG(waitTime, 0));
                    }
                    
                    if (waitTime != data->config->settings.waitTime) {
                        data->config->settings.waitTime = waitTime;
                        data->changed = TRUE;
                        SaveConfig(data->exePath, data->config);
                    }
                }
                return 0;
            }
            
            // Handle WaitTime spin control
            if (LOWORD(wParam) == ID_WAITTIME_SPIN) {
                // Spin control changes are handled through the buddy edit control
                return 0;
            }
            
            switch (LOWORD(wParam)) {
                case ID_COMMAND_LIST:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        // Double click - edit
                        if (sel != LB_ERR && sel < data->config->commandCount) {
                            if (ShowCommandEditor(hwnd, &data->config->commands[sel], FALSE)) {
                                data->changed = TRUE;
                                SaveConfig(data->exePath, data->config);
                                RefreshSettingsList(data);
                                SendMessage(data->listBox, LB_SETCURSEL, sel, 0);
                            }
                        }
                    }
                    break;
                    
                case ID_ADD_BTN: {
                    if (data->config->commandCount >= MAX_COMMANDS) {
                        MessageBoxW(hwnd, I18n_GetStringW(IDS_MAX_COMMANDS_WARNING), 
                                   I18n_GetStringW(IDS_WARNING), MB_OK | MB_ICONWARNING);
                        break;
                    }
                    
                    Command newCmd;
                    ZeroMemory(&newCmd, sizeof(newCmd));
                    strcpy(newCmd.arguments, "{url}");
                    
                    if (ShowCommandEditor(hwnd, &newCmd, TRUE)) {
                        data->config->commands[data->config->commandCount] = newCmd;
                        data->config->commandCount++;
                        data->changed = TRUE;
                        SaveConfig(data->exePath, data->config);
                        RefreshSettingsList(data);
                        SendMessage(data->listBox, LB_SETCURSEL, data->config->commandCount - 1, 0);
                    }
                    SetFocus(data->listBox);
                    break;
                }
                
                case ID_EDIT_BTN: {
                    if (sel != LB_ERR && sel < data->config->commandCount) {
                        if (ShowCommandEditor(hwnd, &data->config->commands[sel], FALSE)) {
                            data->changed = TRUE;
                            SaveConfig(data->exePath, data->config);
                            RefreshSettingsList(data);
                            SendMessage(data->listBox, LB_SETCURSEL, sel, 0);
                        }
                    }
                    SetFocus(data->listBox);
                    break;
                }
                
                case ID_DELETE_BTN: {
                    if (sel != LB_ERR && sel < data->config->commandCount) {
                        wchar_t msg[512];
                        wchar_t wideName[MAX_NAME_LENGTH];
                        MultiByteToWideChar(CP_ACP, 0, data->config->commands[sel].name, -1, wideName, MAX_NAME_LENGTH);
                        
                        swprintf(msg, sizeof(msg)/sizeof(wchar_t), I18n_GetStringW(IDS_CONFIRM_DELETE_MSG), wideName);
                        
                        if (MessageBoxW(hwnd, msg, I18n_GetStringW(IDS_CONFIRM_DELETE_TITLE), 
                                       MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            // Shift remaining commands
                            for (int i = sel; i < data->config->commandCount - 1; i++) {
                                data->config->commands[i] = data->config->commands[i + 1];
                            }
                            data->config->commandCount--;
                            
                            // Adjust default index
                            if (data->config->settings.defaultCommandIndex >= data->config->commandCount) {
                                data->config->settings.defaultCommandIndex = data->config->commandCount > 0 ? data->config->commandCount - 1 : 0;
                            } else if (data->config->settings.defaultCommandIndex > sel) {
                                data->config->settings.defaultCommandIndex--;
                            }
                            
                            data->changed = TRUE;
                            SaveConfig(data->exePath, data->config);
                            RefreshSettingsList(data);
                            
                            if (sel >= data->config->commandCount) sel = data->config->commandCount - 1;
                            if (sel >= 0) SendMessage(data->listBox, LB_SETCURSEL, sel, 0);
                        }
                    }
                    SetFocus(data->listBox);
                    break;
                }
                
                case ID_MOVE_UP_BTN: {
                    if (sel > 0 && sel < data->config->commandCount) {
                        SwapCommands(data->config, sel, sel - 1);
                        data->changed = TRUE;
                        SaveConfig(data->exePath, data->config);
                        RefreshSettingsList(data);
                        SendMessage(data->listBox, LB_SETCURSEL, sel - 1, 0);
                    }
                    SetFocus(data->listBox);
                    break;
                }
                
                case ID_MOVE_DOWN_BTN: {
                    if (sel >= 0 && sel < data->config->commandCount - 1) {
                        SwapCommands(data->config, sel, sel + 1);
                        data->changed = TRUE;
                        SaveConfig(data->exePath, data->config);
                        RefreshSettingsList(data);
                        SendMessage(data->listBox, LB_SETCURSEL, sel + 1, 0);
                    }
                    SetFocus(data->listBox);
                    break;
                }
                
                case ID_SET_DEFAULT_BTN: {
                    if (sel != LB_ERR && sel < data->config->commandCount) {
                        data->config->settings.defaultCommandIndex = sel;
                        data->changed = TRUE;
                        SaveConfig(data->exePath, data->config);
                        RefreshSettingsList(data);
                        SendMessage(data->listBox, LB_SETCURSEL, sel, 0);
                    }
                    SetFocus(data->listBox);
                    break;
                }
                
                case ID_CLOSE_BTN:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        
        case WM_CLOSE: {
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

BOOL ShowSettingsWindow(HWND parent, Configuration* config, const char* exePath) {
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    
    // Register window class
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SettingsWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SettingsWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassExW(&wc);
    
    // Prepare data
    SettingsData data;
    data.config = config;
    data.exePath = exePath;
    data.changed = FALSE;
    
    // Calculate position
    RECT parentRect;
    GetWindowRect(parent, &parentRect);
    int winWidth = 410;
    int winHeight = 290;
    int winX = parentRect.left + (parentRect.right - parentRect.left - winWidth) / 2;
    int winY = parentRect.top + (parentRect.bottom - parentRect.top - winHeight) / 2;
    
    // Create window
    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"SettingsWindowClass",
        I18n_GetStringW(IDS_SETTINGS_TITLE),
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
    
    return data.changed;
}
