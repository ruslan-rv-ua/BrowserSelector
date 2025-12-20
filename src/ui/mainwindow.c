#include "mainwindow.h"
#include "settings.h"
#include "../executor/executor.h"
#include "../registry/registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>

#define ID_LISTBOX 1001
#define ID_REGISTER_BTN 1002
#define ID_SETTINGS_BTN 1003

#define MAIN_WINDOW_WIDTH 400
#define MAIN_WINDOW_HEIGHT 300

static const char* MAIN_WINDOW_CLASS = "BrowserSelectorClass";
static WNDPROC originalListBoxProc = NULL;
static WNDPROC originalRegisterButtonProc = NULL;
static WNDPROC originalSettingsButtonProc = NULL;

// Subclass procedure for Settings button to intercept Tab key
LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* mainWin = (MainWindow*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

    if (msg == WM_KEYDOWN && mainWin) {
        if (wParam == VK_TAB) {
            BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

            // Tab cycle: ListBox -> Register -> Settings -> ListBox
            if (hwnd == mainWin->registerBtn) {
                if (shift) {
                    SetFocus(mainWin->listBox);
                } else {
                    SetFocus(mainWin->settingsBtn);
                }
            } else if (hwnd == mainWin->settingsBtn) {
                if (shift) {
                    SetFocus(mainWin->registerBtn);
                } else {
                    SetFocus(mainWin->listBox);
                }
            } else {
                // Fallback: send focus to list
                SetFocus(mainWin->listBox);
            }

            return 0;
        }

        if (wParam == VK_ESCAPE) {
            DestroyWindow(GetParent(hwnd));
            return 0;
        }
    }

    // Call appropriate original Button procedure
    if (hwnd == mainWin->registerBtn) {
        return CallWindowProc(originalRegisterButtonProc, hwnd, msg, wParam, lParam);
    }
    if (hwnd == mainWin->settingsBtn) {
        return CallWindowProc(originalSettingsButtonProc, hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Subclass procedure for ListBox to intercept key presses
LRESULT CALLBACK ListBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* mainWin = (MainWindow*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
    
    if (msg == WM_KEYDOWN && mainWin) {
        switch (wParam) {
            case VK_RETURN: {
                // Enter - execute selected command
                int index = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR && index < mainWin->config->commandCount) {
                    ExecuteCommand(&mainWin->config->commands[index], mainWin->url);
                    DestroyWindow(GetParent(hwnd));
                }
                return 0;
            }
            
            case VK_ESCAPE:
                DestroyWindow(GetParent(hwnd));
                return 0;
                
            case VK_TAB: {
                // Tab - switch focus to Register button
                BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                if (shift) {
                    // Shift+Tab -> move focus to Settings (reverse cycle)
                    SetFocus(mainWin->settingsBtn);
                } else {
                    SetFocus(mainWin->registerBtn);
                }
                return 0;
            }
        }
    }
    
    if (msg == WM_CHAR && mainWin) {
        // Quick select 1-9
        if (wParam >= '1' && wParam <= '9') {
            int index = wParam - '1';
            if (index < mainWin->config->commandCount) {
                ExecuteCommand(&mainWin->config->commands[index], mainWin->url);
                DestroyWindow(GetParent(hwnd));
            }
            return 0;
        }
    }
    
    // Call original ListBox procedure
    return CallWindowProc(originalListBoxProc, hwnd, msg, wParam, lParam);
}

void RefreshCommandList(MainWindow* mainWin) {
    // Clear list box
    SendMessage(mainWin->listBox, LB_RESETCONTENT, 0, 0);
    
    // Add commands without numbers
    for (int i = 0; i < mainWin->config->commandCount; i++) {
        SendMessageA(mainWin->listBox, LB_ADDSTRING, 0, (LPARAM)mainWin->config->commands[i].name);
    }
    
    // Set default selection
    int sel = mainWin->config->settings.defaultCommandIndex;
    if (sel >= mainWin->config->commandCount) sel = 0;
    SendMessage(mainWin->listBox, LB_SETCURSEL, sel, 0);
}

HWND CreateMainWindow(HINSTANCE hInstance, Configuration* config, const char* url, const char* exePath) {
    // Register window class
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_WINDOW_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    
    if (!RegisterClassExA(&wc)) {
        // Class may already be registered
        DWORD err = GetLastError();
        if (err != ERROR_CLASS_ALREADY_EXISTS) {
            return NULL;
        }
    }
    
    // Create window data structure
    MainWindow* mainWin = (MainWindow*)malloc(sizeof(MainWindow));
    if (!mainWin) return NULL;
    
    ZeroMemory(mainWin, sizeof(MainWindow));
    mainWin->config = config;
    mainWin->url = url ? _strdup(url) : NULL;
    mainWin->hInstance = hInstance;
    strncpy(mainWin->exePath, exePath, MAX_PATH - 1);
    
    // Calculate window position (center screen) using hard-coded constants
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int winX = (screenWidth - MAIN_WINDOW_WIDTH) / 2;
    int winY = (screenHeight - MAIN_WINDOW_HEIGHT) / 2;
    
    // Create window
    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        MAIN_WINDOW_CLASS,
        "Browser Selector",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        winX, winY,
        MAIN_WINDOW_WIDTH,
        MAIN_WINDOW_HEIGHT,
        NULL, NULL, hInstance, mainWin
    );
    
    if (!hwnd) {
        if (mainWin->url) free(mainWin->url);
        free(mainWin);
        return NULL;
    }
    
    return hwnd;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* mainWin = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            mainWin = (MainWindow*)cs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)mainWin);
            mainWin->hwnd = hwnd;
            
            // Get client rect for positioning
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int clientWidth = clientRect.right - clientRect.left;
            int clientHeight = clientRect.bottom - clientRect.top;
            
            // Create ListBox
            mainWin->listBox = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "LISTBOX",
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | 
                LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                10, 10, 
                clientWidth - 20, 
                clientHeight - 60,
                hwnd, (HMENU)ID_LISTBOX, cs->hInstance, NULL
            );
            
            // Set font
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(mainWin->listBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Subclass ListBox to intercept key presses
            originalListBoxProc = (WNDPROC)SetWindowLongPtr(mainWin->listBox, 
                GWLP_WNDPROC, (LONG_PTR)ListBoxSubclassProc);
            
            // Fill ListBox with commands
            RefreshCommandList(mainWin);
            
            // Create buttons (Register and Settings)
            int btnWidth = 100;
            int btnHeight = 30;
            int btnSpacing = 10;
            int totalWidth = btnWidth * 2 + btnSpacing;
            int btnStartX = (clientWidth - totalWidth) / 2;
            int btnY = clientHeight - btnHeight - 10;
            
            // Create Register/Unregister button (left)
            BOOL isRegistered = IsRegisteredAsBrowser();
            BOOL isDefault = IsDefaultBrowser();
            const char* registerBtnText = (isRegistered && isDefault) ? "Unregister" : "Set as Default";
            
            mainWin->registerBtn = CreateWindowExA(
                0,
                "BUTTON",
                registerBtnText,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnStartX, btnY, btnWidth, btnHeight,
                hwnd, (HMENU)ID_REGISTER_BTN, cs->hInstance, NULL
            );
            SendMessage(mainWin->registerBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Create Settings button (right)
            mainWin->settingsBtn = CreateWindowExA(
                0,
                "BUTTON",
                "Settings",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnStartX + btnWidth + btnSpacing, btnY, btnWidth, btnHeight,
                hwnd, (HMENU)ID_SETTINGS_BTN, cs->hInstance, NULL
            );
            SendMessage(mainWin->settingsBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Subclass Register and Settings buttons to intercept Tab key
            originalRegisterButtonProc = (WNDPROC)SetWindowLongPtr(mainWin->registerBtn,
                GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);
            originalSettingsButtonProc = (WNDPROC)SetWindowLongPtr(mainWin->settingsBtn,
                GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);
            
            // Set focus to ListBox
            SetFocus(mainWin->listBox);
            
            return 0;
        }
        
        case WM_COMMAND: {
            if (!mainWin) break;
            
            switch (LOWORD(wParam)) {
                case ID_LISTBOX:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        // Double click - execute command
                        int index = (int)SendMessage(mainWin->listBox, LB_GETCURSEL, 0, 0);
                        if (index != LB_ERR && index < mainWin->config->commandCount) {
                            ExecuteCommand(&mainWin->config->commands[index], mainWin->url);
                            DestroyWindow(hwnd);
                        }
                    }
                    break;
                    
                case ID_REGISTER_BTN: {
                    // Check current state
                    BOOL isRegistered = IsRegisteredAsBrowser();
                    BOOL isDefault = IsDefaultBrowser();
                    
                    if (isRegistered && isDefault) {
                        // Unregister completely
                        if (UnregisterAsBrowser()) {
                            MessageBoxA(hwnd, 
                                "Browser Selector has been unregistered.\n\n"
                                "It will no longer appear in the list of available browsers.",
                                "Unregistration Successful", MB_OK | MB_ICONINFORMATION);
                            SetWindowTextA(mainWin->registerBtn, "Set as Default");
                        } else {
                            MessageBoxA(hwnd, 
                                "Failed to unregister Browser Selector.\n\n"
                                "Please check permissions and try again.",
                                "Unregistration Failed", MB_OK | MB_ICONERROR);
                        }
                    } else {
                        // Register and set as default
                        BOOL success = TRUE;
                        
                        if (!isRegistered) {
                            // First register
                            success = RegisterAsBrowser(mainWin->exePath);
                            if (!success) {
                                MessageBoxA(hwnd, 
                                    "Failed to register Browser Selector.\n\n"
                                    "Try running as administrator.",
                                    "Registration Failed", MB_OK | MB_ICONERROR);
                            }
                        }
                        
                        if (success) {
                            // Try to set as default
                            BOOL setDefault = SetAsDefaultBrowser();
                            
                            if (setDefault) {
                                MessageBoxA(hwnd, 
                                    "Browser Selector has been set as your default browser.\n\n"
                                    "All web links will now open through Browser Selector.",
                                    "Success", MB_OK | MB_ICONINFORMATION);
                                SetWindowTextA(mainWin->registerBtn, "Unregister");
                            } else {
                                // Windows Settings opened - inform user
                                MessageBoxA(hwnd, 
                                    "Browser Selector has been registered.\n\n"
                                    "Windows Settings has been opened where you can manually "
                                    "select Browser Selector as your default browser.\n\n"
                                    "Look for 'BrowserSelector' in the list of web browsers.",
                                    "Manual Selection Required", MB_OK | MB_ICONINFORMATION);
                                // Don't change button text yet - wait for user to complete in Settings
                            }
                        }
                    }
                    SetFocus(mainWin->listBox);
                    break;
                }
                    
                case ID_SETTINGS_BTN:
                    // Open settings window
                    if (ShowSettingsWindow(hwnd, mainWin->config, mainWin->exePath)) {
                        // Settings changed, refresh list
                        RefreshCommandList(mainWin);
                    }
                    SetFocus(mainWin->listBox);
                    break;
            }
            return 0;
        }
        
        case WM_ACTIVATE: {
            if (LOWORD(wParam) != WA_INACTIVE && mainWin && mainWin->listBox) {
                // Use PostMessage to set focus after window activation is complete
                // This ensures keyboard input works correctly after Alt+Tab
                PostMessage(hwnd, WM_USER + 1, 0, 0);
            }
            break;
        }
        
        case WM_USER + 1: {
            // Set focus to ListBox (delayed from WM_ACTIVATE)
            if (mainWin && mainWin->listBox) {
                SetFocus(mainWin->listBox);
            }
            return 0;
        }
        
        case WM_DESTROY: {
            if (mainWin) {
                if (mainWin->url) free(mainWin->url);
                free(mainWin);
            }
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
