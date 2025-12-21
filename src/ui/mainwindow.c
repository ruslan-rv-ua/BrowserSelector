#include "mainwindow.h"
#include "settings.h"
#include "../executor/executor.h"
#include "../registry/registry.h"
#include "../i18n/i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>

#define ID_LISTBOX 1001
#define ID_REGISTER_BTN 1002
#define ID_SETTINGS_BTN 1003
#define ID_COUNTDOWN_LABEL 1004
#define TIMER_ID 1005

#define MAIN_WINDOW_WIDTH 480  // Increased from 400 to accommodate wider buttons
#define MAIN_WINDOW_HEIGHT 320

static const char* MAIN_WINDOW_CLASS = "BrowserSelectorClass";
static WNDPROC originalListBoxProc = NULL;
static WNDPROC originalRegisterButtonProc = NULL;
static WNDPROC originalSettingsButtonProc = NULL;

// Forward declarations for timer functions
static void StartTimer(MainWindow* mainWin);
static void StopTimer(MainWindow* mainWin);
static void UpdateCountdownDisplay(MainWindow* mainWin);
static void ExecuteDefaultCommand(MainWindow* mainWin);

// Subclass procedure for Settings button to intercept Tab key
LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* mainWin = (MainWindow*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

    if (msg == WM_KEYDOWN && mainWin) {
        // Stop timer on any key press
        if (mainWin->timerActive) {
            StopTimer(mainWin);
        }
        
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

        if (wParam == VK_RETURN) {
            // Enter - activate the button
            SendMessage(GetParent(hwnd), WM_COMMAND,
                       (WPARAM)GetWindowLongPtr(hwnd, GWLP_ID), (LPARAM)hwnd);
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
        // Stop timer on any key press
        if (mainWin->timerActive) {
            StopTimer(mainWin);
        }
        
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
        // Stop timer on any character input
        if (mainWin->timerActive) {
            StopTimer(mainWin);
        }
        
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

// Timer functions
static void StartTimer(MainWindow* mainWin) {
    if (mainWin->timerActive || mainWin->config->settings.waitTime <= 0) {
        return;
    }
    
    mainWin->remainingSeconds = mainWin->config->settings.waitTime;
    mainWin->timerActive = TRUE;
    
    // Start timer (1000ms = 1 second)
    mainWin->timerId = SetTimer(mainWin->hwnd, TIMER_ID, 1000, NULL);
    
    // Update countdown display
    wchar_t countdownText[256];
    swprintf(countdownText, 256, I18n_GetStringW(IDS_COUNTDOWN_SECONDS), mainWin->remainingSeconds);
    SetWindowTextW(mainWin->countdownLabel, countdownText);
}

static void StopTimer(MainWindow* mainWin) {
    if (mainWin->timerActive && mainWin->timerId != 0) {
        KillTimer(mainWin->hwnd, mainWin->timerId);
        mainWin->timerId = 0;
        mainWin->timerActive = FALSE;
        mainWin->remainingSeconds = 0;
        
        // Clear countdown display
        SetWindowTextW(mainWin->countdownLabel, L"");
    }
}

static void UpdateCountdownDisplay(MainWindow* mainWin) {
    if (mainWin->timerActive) {
        wchar_t countdownText[256];
        if (mainWin->remainingSeconds > 1) {
            swprintf(countdownText, 256, I18n_GetStringW(IDS_COUNTDOWN_SECONDS), mainWin->remainingSeconds);
        } else if (mainWin->remainingSeconds == 1) {
            wcscpy(countdownText, I18n_GetStringW(IDS_COUNTDOWN_SECOND));
        } else {
            wcscpy(countdownText, I18n_GetStringW(IDS_COUNTDOWN_NOW));
        }
        SetWindowTextW(mainWin->countdownLabel, countdownText);
    }
}

static void ExecuteDefaultCommand(MainWindow* mainWin) {
    int defaultIndex = mainWin->config->settings.defaultCommandIndex;
    if (defaultIndex >= 0 && defaultIndex < mainWin->config->commandCount) {
        ExecuteCommand(&mainWin->config->commands[defaultIndex], mainWin->url);
    }
    DestroyWindow(mainWin->hwnd);
}

HWND CreateMainWindow(HINSTANCE hInstance, Configuration* config, const char* url, const char* exePath) {
    // Register window class
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"BrowserSelectorClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    
    if (!RegisterClassExW(&wc)) {
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
    
    // Initialize timer fields
    mainWin->timerId = 0;
    mainWin->timerActive = FALSE;
    mainWin->remainingSeconds = 0;
    
    // Calculate window position (center screen) using hard-coded constants
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int winX = (screenWidth - MAIN_WINDOW_WIDTH) / 2;
    int winY = (screenHeight - MAIN_WINDOW_HEIGHT) / 2;
    
    // Create window
    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"BrowserSelectorClass",
        I18n_GetStringW(IDS_APP_TITLE),
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
            
            // Create countdown label (positioned above ListBox)
            mainWin->countdownLabel = CreateWindowExA(
                0,
                "STATIC",
                "",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                10, 5,
                clientWidth - 20, 20,
                hwnd, (HMENU)ID_COUNTDOWN_LABEL, cs->hInstance, NULL
            );
            
            // Create ListBox (positioned below countdown label)
            mainWin->listBox = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "LISTBOX",
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP |
                LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                10, 30,
                clientWidth - 20,
                clientHeight - 80,
                hwnd, (HMENU)ID_LISTBOX, cs->hInstance, NULL
            );
            
            // Set font
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(mainWin->listBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(mainWin->countdownLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Subclass ListBox to intercept key presses
            originalListBoxProc = (WNDPROC)SetWindowLongPtr(mainWin->listBox, 
                GWLP_WNDPROC, (LONG_PTR)ListBoxSubclassProc);
            
            // Fill ListBox with commands
            RefreshCommandList(mainWin);
            
            // Create buttons (Register and Settings)
            int btnWidth = 160;  // Increased from 100 to accommodate longer text
            int btnHeight = 30;
            int btnSpacing = 10;
            int totalWidth = btnWidth * 2 + btnSpacing;
            int btnStartX = (clientWidth - totalWidth) / 2;
            int btnY = clientHeight - btnHeight - 10;
            
            // Create Register/Unregister button (left)
            BOOL isRegistered = IsRegisteredAsBrowser();
            BOOL isDefault = IsDefaultBrowser();
            const wchar_t* registerBtnText = (isRegistered && isDefault) ? 
                I18n_GetStringW(IDS_UNREGISTER_BTN) : I18n_GetStringW(IDS_SET_DEFAULT_BTN);
            
            mainWin->registerBtn = CreateWindowExW(
                0,
                L"BUTTON",
                registerBtnText,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                btnStartX, btnY, btnWidth, btnHeight,
                hwnd, (HMENU)ID_REGISTER_BTN, cs->hInstance, NULL
            );
            SendMessage(mainWin->registerBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Create Settings button (right)
            mainWin->settingsBtn = CreateWindowExW(
                0,
                L"BUTTON",
                I18n_GetStringW(IDS_SETTINGS_BTN),
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
            
            // Start auto-open timer if waitTime is configured
            if (mainWin->config->settings.waitTime > 0) {
                StartTimer(mainWin);
            }
            
            return 0;
        }
        
        case WM_COMMAND: {
            if (!mainWin) break;
            
            switch (LOWORD(wParam)) {
                case ID_LISTBOX:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        // Stop timer on user interaction
                        if (mainWin->timerActive) {
                            StopTimer(mainWin);
                        }
                        
                        // Double click - execute command
                        int index = (int)SendMessage(mainWin->listBox, LB_GETCURSEL, 0, 0);
                        if (index != LB_ERR && index < mainWin->config->commandCount) {
                            ExecuteCommand(&mainWin->config->commands[index], mainWin->url);
                            DestroyWindow(hwnd);
                        }
                    }
                    break;
                    
                case ID_REGISTER_BTN: {
                    // Stop timer on user interaction
                    if (mainWin->timerActive) {
                        StopTimer(mainWin);
                    }
                    
                    // Check current state
                    BOOL isRegistered = IsRegisteredAsBrowser();
                    BOOL isDefault = IsDefaultBrowser();
                    
                    if (isRegistered && isDefault) {
                        // Unregister completely
                        if (UnregisterAsBrowser()) {
                            MessageBoxW(hwnd, 
                                I18n_GetStringW(IDS_UNREG_SUCCESS_MSG),
                                I18n_GetStringW(IDS_UNREG_SUCCESS_TITLE), 
                                MB_OK | MB_ICONINFORMATION);
                            SetWindowTextW(mainWin->registerBtn, I18n_GetStringW(IDS_SET_DEFAULT_BTN));
                        } else {
                            MessageBoxW(hwnd, 
                                I18n_GetStringW(IDS_UNREG_FAILED_MSG),
                                I18n_GetStringW(IDS_UNREG_FAILED_TITLE), 
                                MB_OK | MB_ICONERROR);
                        }
                    } else {
                        // Register and set as default
                        BOOL success = TRUE;
                        
                        if (!isRegistered) {
                            // First register
                            success = RegisterAsBrowser(mainWin->exePath);
                            if (!success) {
                                MessageBoxW(hwnd, 
                                    I18n_GetStringW(IDS_REG_FAILED_MSG),
                                    I18n_GetStringW(IDS_REG_FAILED_TITLE), 
                                    MB_OK | MB_ICONERROR);
                            }
                        }
                        
                        if (success) {
                            // Try to set as default (now with automatic hash method)
                            BOOL setDefault = SetAsDefaultBrowser();
                            
                            if (setDefault) {
                                MessageBoxW(hwnd, 
                                    I18n_GetStringW(IDS_REG_SUCCESS_MSG),
                                    I18n_GetStringW(IDS_REG_SUCCESS_TITLE), 
                                    MB_OK | MB_ICONINFORMATION);
                                SetWindowTextW(mainWin->registerBtn, I18n_GetStringW(IDS_UNREGISTER_BTN));
                            } else {
                                // Settings opened - user needs to complete manually
                                int result = MessageBoxW(hwnd,
                                    I18n_GetStringW(IDS_VERIFY_MSG),
                                    I18n_GetStringW(IDS_VERIFY_TITLE),
                                    MB_YESNO | MB_ICONQUESTION);
                                
                                if (result == IDYES) {
                                    // Re-check status
                                    if (IsDefaultBrowser()) {
                                        MessageBoxW(hwnd,
                                            I18n_GetStringW(IDS_NOW_DEFAULT_MSG),
                                            I18n_GetStringW(IDS_SUCCESS),
                                            MB_OK | MB_ICONINFORMATION);
                                        SetWindowTextW(mainWin->registerBtn, I18n_GetStringW(IDS_UNREGISTER_BTN));
                                    } else {
                                        MessageBoxW(hwnd,
                                            I18n_GetStringW(IDS_NOT_DEFAULT_MSG),
                                            I18n_GetStringW(IDS_WARNING),
                                            MB_OK | MB_ICONWARNING);
                                    }
                                }
                            }
                        }
                    }
                    SetFocus(mainWin->listBox);
                    break;
                }
                    
                case ID_SETTINGS_BTN:
                    // Stop timer on user interaction
                    if (mainWin->timerActive) {
                        StopTimer(mainWin);
                    }
                    
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
        
        case WM_TIMER: {
            if (wParam == TIMER_ID && mainWin && mainWin->timerActive) {
                mainWin->remainingSeconds--;
                
                if (mainWin->remainingSeconds <= 0) {
                    // Timer reached zero - execute default command
                    StopTimer(mainWin);
                    ExecuteDefaultCommand(mainWin);
                } else {
                    // Update countdown display
                    UpdateCountdownDisplay(mainWin);
                }
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
            // Clean up timer
            if (mainWin && mainWin->timerActive) {
                StopTimer(mainWin);
            }
            
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
