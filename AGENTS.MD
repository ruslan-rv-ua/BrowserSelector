# Технічна специфікація: Windows Browser Selector (C Implementation)

## Огляд проєкту
Створення портативного Windows desktop застосунку, який виступає як селектор браузерів. Коли відкривається URL, застосунок показує меню з налаштованими браузерами/командами для вибору користувачем.

## Основні вимоги

### 1. Технологічний стек
- **Мова:** C (стандарт C11)
- **Компілятор:** GCC (MinGW-w64) 8.1.0 або новіший
- **GUI Framework:** Win32 API (нативні Windows контроли)
- **Формат даних:** JSON (бібліотека cJSON)
- **Білд:** Один портативний .exe файл
- **Windows Registry:** Native Win32 Registry API

### 2. Встановлення компонентів

#### 2.1 Встановлення MinGW-w64

**Варіант 1: MSYS2 (рекомендовано)**
```bash
# Завантажити MSYS2 з https://www.msys2.org/
# Встановити MSYS2 в C:\msys64

# Відкрити MSYS2 MINGW64 terminal
pacman -Syu
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
pacman -S git

# Додати до PATH: C:\msys64\mingw64\bin
# Через System Properties -> Advanced -> Environment Variables
# Або через командний рядок (тимчасово):
set PATH=%PATH%;C:\msys64\mingw64\bin
```

**Варіант 2: Standalone MinGW-w64**
```
1. Завантажити з https://winlibs.com/
2. Вибрати: GCC 13.x + MinGW-w64 x86_64
3. Розпакувати в C:\mingw64
4. Додати C:\mingw64\bin до системного PATH:
   - Win+R -> sysdm.cpl -> Advanced -> Environment Variables
   - Знайти PATH -> Edit -> New -> додати C:\mingw64\bin
```

**Перевірка встановлення:**
```bash
gcc --version
# Має вивести: gcc (GCC) 13.x.x або новіше
```

#### 2.1.1 Перевірка встановлення в Windows CMD
```cmd
# Перевірити версію GCC
gcc --version

# Перевірити наявність make
make --version

# Перевірити наявність windres
windres --version
```

#### 2.1.2 Налаштування середовища Windows
```cmd
# Тимчасове налаштування PATH для поточної сесії
set PATH=%PATH%;C:\msys64\mingw64\bin

# Постійне налаштування через системні змінні:
# 1. Win+R -> sysdm.cpl -> Advanced -> Environment Variables
# 2. Знайти PATH в System variables -> Edit -> New
# 3. Додати: C:\msys64\mingw64\bin
# 4. Перезапустити командний рядок
```

#### 2.2 Встановлення cJSON бібліотеки

**Варіант 1: Компіляція з вихідників (рекомендовано)**
```bash
# Клонувати репозиторій
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON

# Компілювати
mkdir build
cd build
gcc -c ../cJSON.c -o cJSON.o
ar rcs libcjson.a cJSON.o

# Копіювати файли в проєкт
# cJSON.h -> include/
# libcjson.a -> lib/
```

**Варіант 2: MSYS2**
```bash
pacman -S mingw-w64-x86_64-cjson
```

#### 2.3 Структура проєкту
```
browserselector/
├── src/
│   ├── main.c              # Точка входу
│   ├── ui/
│   │   ├── mainwindow.c    # Головне вікно
│   │   ├── mainwindow.h
│   │   ├── settings.c      # Вікно налаштувань
│   │   ├── settings.h
│   │   ├── commandeditor.c # Діалог додавання/редагування
│   │   └── commandeditor.h
│   ├── config/
│   │   ├── config.c        # Робота з конфігурацією
│   │   └── config.h
│   ├── executor/
│   │   ├── executor.c      # Запуск команд
│   │   └── executor.h
│   └── registry/
│       ├── registry.c      # Реєстрація браузера
│       └── registry.h
├── include/
│   └── cJSON.h            # Заголовок cJSON
├── lib/
│   └── libcjson.a         # Статична бібліотека cJSON
├── resources/
│   ├── app.rc             # Ресурсний файл
│   └── manifest.xml       # Маніфест застосунку
├── Makefile               # Файл збірки
├── config.json            # Конфігурація за замовчуванням
└── README.md
```

### 3. Архітектура застосунку

#### 3.1 Поведінка точки входу
- Приймає URL як аргумент командного рядка: `BrowserSelector.exe "https://example.com"`
- Якщо URL не надано, браузер відкривається без URL
- Завантажує конфігурацію з `config.json` (в тій же директорії що й .exe)
- Відображає вікно вибору команди

#### 3.2 Структура конфігураційного файлу
**Файл:** `config.json` (зберігається поряд з .exe)

```json
{
  "settings": {
    "defaultCommandIndex": 0
  },
  "commands": [
    {
      "name": "Chrome",
      "command": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
      "arguments": "{url}"
    },
    {
      "name": "Firefox",
      "command": "C:\\Program Files\\Mozilla Firefox\\firefox.exe",
      "arguments": "{url}"
    },
    {
      "name": "Edge",
      "command": "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
      "arguments": "{url}"
    },
    {
      "name": "LibreWolf",
      "command": "C:\\Program Files\\LibreWolf\\librewolf.exe",
      "arguments": "{url}"
    },
    {
      "name": "Opera",
      "command": "C:\\Program Files\\Opera\\launcher.exe",
      "arguments": "{url}"
    }
  ]
}
```

### 4. Структури даних

#### 4.1 Основні структури (config/config.h)
```c
#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PATH_LENGTH 260
#define MAX_NAME_LENGTH 100
#define MAX_COMMANDS 50

typedef struct {
    char name[MAX_NAME_LENGTH];
    char command[MAX_PATH_LENGTH];
    char arguments[MAX_PATH_LENGTH];
} Command;

typedef struct {
    int defaultCommandIndex;
} Settings;

typedef struct {
    Settings settings;
    Command commands[MAX_COMMANDS];
    int commandCount;
} Configuration;

// Функції для роботи з конфігурацією
int LoadConfig(const char* exePath, Configuration* config);
int SaveConfig(const char* exePath, const Configuration* config);
int CreateDefaultConfig(const char* configPath);

#endif // CONFIG_H
```

#### 4.2 UI структури (ui/mainwindow.h)
```c
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include "../config/config.h"

typedef struct {
    HWND hwnd;
    HWND listBox;
    HWND settingsBtn;
    Configuration* config;
    char* url;
} MainWindow;

HWND CreateMainWindow(HINSTANCE hInstance, Configuration* config, const char* url);
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // MAINWINDOW_H
```

### 5. Реалізація основних компонентів

#### 5.1 Точка входу (src/main.c)
```c
#include <windows.h>
#include <stdio.h>
#include "config/config.h"
#include "ui/mainwindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // Отримати шлях до executable
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    // Завантажити конфігурацію
    Configuration config;
    if (!LoadConfig(exePath, &config)) {
        MessageBoxA(NULL, "Failed to load configuration", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Отримати URL з командного рядка
    char* url = NULL;
    if (lpCmdLine && strlen(lpCmdLine) > 0) {
        // Прибрати лапки якщо є
        if (lpCmdLine[0] == '"') {
            url = lpCmdLine + 1;
            char* end = strrchr(url, '"');
            if (end) *end = '\0';
        } else {
            url = lpCmdLine;
        }
    }
    
    // Створити головне вікно
    HWND hwnd = CreateMainWindow(hInstance, &config, url);
    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Цикл обробки повідомлень
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
```

#### 5.2 Робота з конфігурацією (src/config/config.c)
```c
#include "config.h"
#include "../../include/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

int LoadConfig(const char* exePath, Configuration* config) {
    // Отримати директорію exe
    char configPath[MAX_PATH];
    strcpy(configPath, exePath);
    char* lastSlash = strrchr(configPath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
        strcat(configPath, "config.json");
    }
    
    // Спробувати відкрити файл
    FILE* file = fopen(configPath, "r");
    if (!file) {
        // Створити конфігурацію за замовчуванням
        return CreateDefaultConfig(configPath);
    }
    
    // Прочитати файл
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* jsonString = (char*)malloc(fileSize + 1);
    fread(jsonString, 1, fileSize, file);
    jsonString[fileSize] = '\0';
    fclose(file);
    
    // Парсити JSON
    cJSON* json = cJSON_Parse(jsonString);
    free(jsonString);
    
    if (!json) {
        return 0;
    }
    
    // Зчитати settings
    cJSON* settings = cJSON_GetObjectItem(json, "settings");
    if (settings) {
        cJSON* defaultIndex = cJSON_GetObjectItem(settings, "defaultCommandIndex");
        
        config->settings.defaultCommandIndex = defaultIndex ? defaultIndex->valueint : 0;
    }
    
    // Зчитати commands
    cJSON* commands = cJSON_GetObjectItem(json, "commands");
    if (commands) {
        config->commandCount = 0;
        cJSON* command = NULL;
        cJSON_ArrayForEach(command, commands) {
            if (config->commandCount >= MAX_COMMANDS) break;
            
            cJSON* name = cJSON_GetObjectItem(command, "name");
            cJSON* cmd = cJSON_GetObjectItem(command, "command");
            cJSON* args = cJSON_GetObjectItem(command, "arguments");
            
            if (name && cmd && args) {
                strncpy(config->commands[config->commandCount].name, 
                       name->valuestring, MAX_NAME_LENGTH - 1);
                strncpy(config->commands[config->commandCount].command, 
                       cmd->valuestring, MAX_PATH_LENGTH - 1);
                strncpy(config->commands[config->commandCount].arguments, 
                       args->valuestring, MAX_PATH_LENGTH - 1);
                config->commandCount++;
            }
        }
    }
    
    cJSON_Delete(json);
    return 1;
}

int SaveConfig(const char* exePath, const Configuration* config) {
    // Отримати шлях до config.json
    char configPath[MAX_PATH];
    strcpy(configPath, exePath);
    char* lastSlash = strrchr(configPath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
        strcat(configPath, "config.json");
    }
    
    // Створити JSON об'єкт
    cJSON* json = cJSON_CreateObject();
    
    // Додати settings
    cJSON* settings = cJSON_CreateObject();
    cJSON_AddNumberToObject(settings, "defaultCommandIndex", 
                           config->settings.defaultCommandIndex);
    cJSON_AddItemToObject(json, "settings", settings);
    
    // Додати commands
    cJSON* commands = cJSON_CreateArray();
    for (int i = 0; i < config->commandCount; i++) {
        cJSON* command = cJSON_CreateObject();
        cJSON_AddStringToObject(command, "name", config->commands[i].name);
        cJSON_AddStringToObject(command, "command", config->commands[i].command);
        cJSON_AddStringToObject(command, "arguments", config->commands[i].arguments);
        cJSON_AddItemToArray(commands, command);
    }
    cJSON_AddItemToObject(json, "commands", commands);
    
    // Записати в файл
    char* jsonString = cJSON_Print(json);
    FILE* file = fopen(configPath, "w");
    if (file) {
        fputs(jsonString, file);
        fclose(file);
    }
    
    cJSON_free(jsonString);
    cJSON_Delete(json);
    
    return file != NULL;
}

int CreateDefaultConfig(const char* configPath) {
    Configuration config;
    config.settings.defaultCommandIndex = 0;
    config.commandCount = 3;
    
    strcpy(config.commands[0].name, "Chrome");
    strcpy(config.commands[0].command, "chrome.exe");
    strcpy(config.commands[0].arguments, "{url}");
    
    strcpy(config.commands[1].name, "Firefox");
    strcpy(config.commands[1].command, "firefox.exe");
    strcpy(config.commands[1].arguments, "{url}");
    
    strcpy(config.commands[2].name, "Edge");
    strcpy(config.commands[2].command, "msedge.exe");
    strcpy(config.commands[2].arguments, "{url}");
    
    return SaveConfig(configPath, &config);
}
```

#### 5.3 Головне вікно (src/ui/mainwindow.c)
```c
#include "mainwindow.h"
#include "../executor/executor.h"
#include "settings.h"

#define ID_LISTBOX 1001
#define ID_SETTINGS_BTN 1002

HWND CreateMainWindow(HINSTANCE hInstance, Configuration* config, const char* url) {
    // Реєстрація класу вікна
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "BrowserSelectorClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassExA(&wc);
    
    // Створити структуру вікна
    MainWindow* mainWin = (MainWindow*)malloc(sizeof(MainWindow));
    mainWin->config = config;
    mainWin->url = url ? _strdup(url) : NULL;
    
    // Створити вікно
    HWND hwnd = CreateWindowExA(
        0,
        "BrowserSelectorClass",
        "Browser Selector",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400,
        300,
        NULL, NULL, hInstance, mainWin
    );
    
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
            
            // Створити ListBox
            mainWin->listBox = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "LISTBOX",
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                10, 10, 360, 200,
                hwnd, (HMENU)ID_LISTBOX, cs->hInstance, NULL
            );
            
            // Заповнити ListBox командами
            for (int i = 0; i < mainWin->config->commandCount; i++) {
                SendMessageA(mainWin->listBox, LB_ADDSTRING, 0, 
                           (LPARAM)mainWin->config->commands[i].name);
            }
            
            // Встановити вибір за замовчуванням
            SendMessage(mainWin->listBox, LB_SETCURSEL, 
                       mainWin->config->settings.defaultCommandIndex, 0);
            
            // Створити кнопку Settings
            mainWin->settingsBtn = CreateWindowExA(
                0,
                "BUTTON",
                "Settings",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                140, 220, 100, 30,
                hwnd, (HMENU)ID_SETTINGS_BTN, cs->hInstance, NULL
            );
            
            // Встановити фокус на ListBox
            SetFocus(mainWin->listBox);
            
            break;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_LISTBOX) {
                if (HIWORD(wParam) == LBN_DBLCLK) {
                    // Подвійний клік - виконати команду
                    int index = (int)SendMessage(mainWin->listBox, LB_GETCURSEL, 0, 0);
                    if (index != LB_ERR) {
                        ExecuteCommand(&mainWin->config->commands[index], 
                                     mainWin->url);
                        DestroyWindow(hwnd);
                    }
                }
            } else if (LOWORD(wParam) == ID_SETTINGS_BTN) {
                // Відкрити вікно налаштувань
                ShowSettingsWindow(hwnd, mainWin->config);
            }
            break;
        }
        
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_RETURN: {
                    // Enter - виконати вибрану команду
                    HWND focused = GetFocus();
                    if (focused == mainWin->listBox) {
                        int index = (int)SendMessage(mainWin->listBox, LB_GETCURSEL, 0, 0);
                        if (index != LB_ERR) {
                            ExecuteCommand(&mainWin->config->commands[index], 
                                         mainWin->url);
                            DestroyWindow(hwnd);
                        }
                    }
                    break;
                }
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
                case '1': case '2': case '3': case '4': case '5':
                case '6': case '7': case '8': case '9': {
                    // Швидкий вибір 1-9
                    int index = wParam - '1';
                    if (index < mainWin->config->commandCount) {
                        ExecuteCommand(&mainWin->config->commands[index], 
                                     mainWin->url);
                        DestroyWindow(hwnd);
                    }
                    break;
                }
            }
            break;
        }
        
        case WM_DESTROY:
            if (mainWin) {
                if (mainWin->url) free(mainWin->url);
                free(mainWin);
            }
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}
```

#### 5.4 Виконувач команд (src/executor/executor.c)
```c
#include "executor.h"
#include <string.h>
#include <windows.h>

static char* ReplaceUrlPlaceholder(const char* arguments, const char* url) {
    if (!url) url = "";
    
    // Підрахувати розмір результату
    const char* pos = strstr(arguments, "{url}");
    if (!pos) {
        return _strdup(arguments);
    }
    
    size_t resultSize = strlen(arguments) - 5 + strlen(url) + 1;
    char* result = (char*)malloc(resultSize);
    
    // Скопіювати частину до {url}
    size_t prefixLen = pos - arguments;
    strncpy(result, arguments, prefixLen);
    result[prefixLen] = '\0';
    
    // Додати URL
    strcat(result, url);
    
    // Додати частину після {url}
    strcat(result, pos + 5);
    
    return result;
}

static char* ResolveCommand(const char* command) {
    char resolved[MAX_PATH];
    
    // Перевірити чи це абсолютний шлях
    if (strchr(command, ':') || command[0] == '\\') {
        DWORD attrs = GetFileAttributesA(command);
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            return _strdup(command);
        }
        return NULL;
    }
    
    // Перевірити чи це відносний шлях
    if (strchr(command, '\\') || strchr(command, '/')) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            sprintf(resolved, "%s\\%s", exePath, command);
            
            DWORD attrs = GetFileAttributesA(resolved);
            if (attrs != INVALID_FILE_ATTRIBUTES) {
                return _strdup(resolved);
            }
        }
        return NULL;
    }
    
    // Шукати в PATH
    if (SearchPathA(NULL, command, ".exe", MAX_PATH, resolved, NULL)) {
        return _strdup(resolved);
    }
    
    return NULL;
}

int ExecuteCommand(const Command* command, const char* url) {
    // Підготувати аргументи
    char* args = ReplaceUrlPlaceholder(command->arguments, url);
    
    // Визначити шлях до команди
    char* cmdPath = ResolveCommand(command->command);
    if (!cmdPath) {
        char error[512];
        sprintf(error, "Command not found: %s", command->command);
        MessageBoxA(NULL, error, "Error", MB_OK | MB_ICONERROR);
        free(args);
        return 0;
    }
    
    // Підготувати command line
    char cmdLine[2048];
    sprintf(cmdLine, "\"%s\" %s", cmdPath, args);
    
    // Запустити процес
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    BOOL success = CreateProcessA(
        NULL,           // lpApplicationName
        cmdLine,        // lpCommandLine
        NULL,           // lpProcessAttributes
        NULL,           // lpThreadAttributes
        FALSE,          // bInheritHandles
        0,              // dwCreationFlags
        NULL,           // lpEnvironment
        NULL,           // lpCurrentDirectory
        &si,            // lpStartupInfo
        &pi             // lpProcessInformation
    );
    
    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        char error[512];
        sprintf(error, "Failed to launch: %s\nError code: %lu", 
                command->command, GetLastError());
        MessageBoxA(NULL, error, "Error", MB_OK | MB_ICONERROR);
    }
    
    free(cmdPath);
    free(args);
    
    return success;
}
```

### 6. Компіляція та збірка

#### 6.1 Makefile
```makefile
# Compiler settings
CC = gcc
WINDRES = windres
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -mwindows -static-libgcc -static
LIBS = -lcomctl32 -ladvapi32 -lshell32 -luser32 -lgdi32 -lkernel32 -Llib -lcjson

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/config/config.c \
          $(SRC_DIR)/ui/mainwindow.c \
          $(SRC_DIR)/ui/settings.c \
          $(SRC_DIR)/ui/commandeditor.c \
          $(SRC_DIR)/executor/executor.c \
          $(SRC_DIR)/registry/registry.c

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
RESOURCE_OBJ = $(OBJ_DIR)/app.res

# Target executable
TARGET = $(BIN_DIR)/BrowserSelector.exe

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(OBJ_DIR)\config" mkdir "$(OBJ_DIR)\config"
	@if not exist "$(OBJ_DIR)\ui" mkdir "$(OBJ_DIR)\ui"
	@if not exist "$(OBJ_DIR)\executor" mkdir "$(OBJ_DIR)\executor"
	@if not exist "$(OBJ_DIR)\registry" mkdir "$(OBJ_DIR)\registry"
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile resource file
$(RESOURCE_OBJ): resources/app.rc
	$(WINDRES) -i resources/app.rc -o $(RESOURCE_OBJ)

# Link executable
$(TARGET): $(OBJECTS) $(RESOURCE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo Build complete: $(TARGET)

# Clean build files
clean:
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"
	@echo Clean complete

# Release build (optimized, stripped)
release: CFLAGS += -DNDEBUG
release: LDFLAGS += -s
release: clean all
	@echo Release build complete

# Run the application
run: all
	$(TARGET)

.PHONY: all clean release run directories
```

#### 6.2 Ресурсний файл (resources/app.rc)
```rc
#include <windows.h>

// Application icon
1 ICON "app.ico"

// Version information
VS_VERSION_INFO VERSIONINFO
FILEVERSION     1,0,0,0
PRODUCTVERSION  1,0,0,0
FILEFLAGSMASK   0x3fL
FILEFLAGS       0x0L
FILEOS          VOS_NT_WINDOWS32
FILETYPE        VFT_APP
FILESUBTYPE     VFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName",      "Your Company\0"
            VALUE "FileDescription",  "Browser Selector\0"
            VALUE "FileVersion",      "1.0.0.0\0"
            VALUE "InternalName",     "BrowserSelector\0"
            VALUE "LegalCopyright",   "Copyright (C) 2025\0"
            VALUE "OriginalFilename", "BrowserSelector.exe\0"
            VALUE "ProductName",      "Browser Selector\0"
            VALUE "ProductVersion",   "1.0.0.0\0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END

// Application manifest
1 RT_MANIFEST "manifest.xml"
```

#### 6.3 Маніфест (resources/manifest.xml)
```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <assemblyIdentity
    version="1.0.0.0"
    processorArchitecture="*"
    name="BrowserSelector"
    type="win32"/>
  <description>Browser Selector Application</description>
  <dependency>
    <dependentAssembly>
      <assemblyIdentity
        type="win32"
        name="Microsoft.Windows.Common-Controls"
        version="6.0.0.0"
        processorArchitecture="*"
        publicKeyToken="6595b64144ccf1df"
        language="*"/>
    </dependentAssembly>
  </dependency>
  <application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings>
      <dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true</dpiAware>
      <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2</dpiAwareness>
    </windowsSettings>
  </application>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
      </requestedPrivileges>
    </security>
  </trustInfo>
</assembly>
```

### 7. Інструкції по компіляції

#### 7.1 Базова компіляція
```cmd
:: Збірка проєкту
make

:: Збірка release версії (оптимізована, без debug символів)
make release

:: Очистити build файли
make clean

:: Збірка та запуск
make run
```

#### 7.2 Ручна компіляція (без Makefile)
```bash
# Компілювати всі .c файли
gcc -c -Iinclude src/main.c -o obj/main.o
gcc -c -Iinclude src/config/config.c -o obj/config.o
gcc -c -Iinclude src/ui/mainwindow.c -o obj/mainwindow.o
gcc -c -Iinclude src/ui/settings.c -o obj/settings.o
gcc -c -Iinclude src/ui/commandeditor.c -o obj/commandeditor.o
gcc -c -Iinclude src/executor/executor.c -o obj/executor.o
gcc -c -Iinclude src/registry/registry.c -o obj/registry.o

# Скомпілювати ресурси
windres -i resources/app.rc -o obj/app.res

# Зібрати executable
gcc -mwindows -static-libgcc -static -o bin/BrowserSelector.exe \
    obj/main.o obj/config.o obj/mainwindow.o obj/settings.o \
    obj/commandeditor.o obj/executor.o obj.registry.o obj/app.res \
    -Llib -lcjson -lcomctl32 -ladvapi32 -lshell32 -luser32 -lgdi32 -lkernel32

# Додатково: strip для зменшення розміру
strip bin/BrowserSelector.exe
```

#### 7.3 Оптимізації розміру
```cmd
:: Компіляція з максимальною оптимізацією розміру
gcc -Os -s -mwindows -static-libgcc -static -Iinclude ^
    src/main.c src/config/*.c src/ui/*.c ^
    src/executor/*.c src/registry/*.c ^
    obj/app.res -o bin/BrowserSelector.exe ^
    -Llib -lcjson -lcomctl32 -ladvapi32 -lshell32 -luser32 -lgdi32 -lkernel32

:: Стиснення UPX (опціонально)
upx --best --lzma bin/BrowserSelector.exe
```

### 8. Тестування

#### 8.1 Підготовка тестового середовища Windows
```cmd
# Переконатись що всі інструменти доступні
where gcc
where make
where windres

# Перевірити структуру проєкту
dir /s src\
dir include\
dir lib\
dir resources\
```

#### 8.2 Тестові команди
```cmd
:: Тест без URL
bin\BrowserSelector.exe

:: Тест з простим URL
bin\BrowserSelector.exe "https://google.com"

:: Тест з URL що містить спецсимволи
bin\BrowserSelector.exe "https://example.com/search?q=test&lang=uk"

:: Тест з довгим URL
bin\BrowserSelector.exe "https://example.com/very/long/path/with/many/segments?param1=value1&param2=value2"

:: Тест з локальним файлом
bin\BrowserSelector.exe "file:///C:/Users/Test/Desktop/index.html"

:: Тест з URL що містить пробіли
bin\BrowserSelector.exe "https://example.com/search?q=test query"
```

#### 8.3 Перевірка конфігурації
```json
// Тестовий config.json з різними типами шляхів
{
  "settings": {
    "defaultCommandIndex": 0
  },
  "commands": [
    {
      "name": "Chrome (PATH)",
      "command": "chrome.exe",
      "arguments": "{url}"
    },
    {
      "name": "Firefox (absolute)",
      "command": "C:\\Program Files\\Mozilla Firefox\\firefox.exe",
      "arguments": "{url}"
    },
    {
      "name": "Custom (relative)",
      "command": "..\\browsers\\custom.exe",
      "arguments": "{url}"
    }
  ]
}
```

### 9. Чеклист функціональності

- [ ] Компіляція без помилок
- [ ] Запуск з URL аргументом
- [ ] Запуск без URL
- [ ] Завантаження config.json
- [ ] Створення конфігурації за замовчуванням
- [ ] Відображення головного вікна
- [ ] Вибір команди мишею
- [ ] Вибір команди клавіатурою (Enter)
- [ ] Навігація стрілками
- [ ] Швидкий вибір 1-9
- [ ] Відкриття вікна налаштувань
- [ ] Додавання нової команди
- [ ] Редагування команди
- [ ] Видалення команди
- [ ] Зміна порядку команд
- [ ] Збереження налаштувань
- [ ] Виконання команди з PATH
- [ ] Виконання команди з абсолютним шляхом
- [ ] Виконання команди з відносним шляхом
- [ ] Обробка помилок (команда не знайдена)
- [ ] Робота з Unicode шляхами
- [ ] Обробка спеціальних символів в URL

### 10. Поширені проблеми та рішення

#### 10.1 Проблеми з встановленням Windows

**Проблема:** Команди gcc, make не розпізнаються
```cmd
# Рішення: Перевірити PATH
echo %PATH%

# Якщо шлях відсутній, додати тимчасово:
set PATH=%PATH%;C:\msys64\mingw64\bin

# Або постійно через System Properties
```

**Проблема:** Помилка "cannot find -lcjson"
```cmd
# Рішення: Перевірити наявність бібліотеки
dir lib\libcjson.a

# Якщо відсутня, скомпілювати cJSON:
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON
mkdir build
cd build
gcc -c ../cJSON.c -o cJSON.o
ar rcs libcjson.a cJSON.o
copy ..\cJSON.h ..\..\include\
copy libcjson.a ..\..\lib\
```

#### 10.2 Помилки компіляції

**Проблема:** `cJSON.h: No such file or directory`
```bash
# Рішення: Перевірити шляхи до include
gcc -Iinclude ...
# Або скопіювати cJSON.h в include/
```

**Проблема:** `undefined reference to 'cJSON_Parse'`
```bash
# Рішення: Додати -lcjson до лінкування
gcc ... -Llib -lcjson
```

**Проблема:** Вікно консолі з'являється
```bash
# Рішення: Використовувати -mwindows
gcc -mwindows ...
```

#### 10.3 Помилки виконання

**Проблема:** "Failed to load configuration"
- Перевірити чи config.json в тій же директорії що й .exe
- Перевірити синтаксис JSON
- Перевірити права доступу до файлу

**Проблема:** "Command not found"
- Перевірити чи команда в PATH
- Використовувати абсолютний шлях
- Перевірити чи існує файл

### 11. Додаткові можливості

#### 11.1 Іконка застосунку
```cmd
# Створити .ico файл з PNG
# Варіант 1: ImageMagick (потрібно встановити)
magick convert icon.png -define icon:auto-resize=256,128,64,48,32,16 app.ico

# Варіант 2: Онлайн сервіс
# https://convertio.co/png-ico/ або https://www.icoconverter.com/

# Варіант 3: GIMP (безкоштовний редактор)
# Відкрити PNG -> File -> Export As -> вибрати .ico формат
```

#### 11.2 Цифровий підпис
```cmd
# Використовувати signtool.exe з Windows SDK
# Зазвичай знаходиться в C:\Program Files (x86)\Windows Kits\10\bin\<version>\x64\
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com BrowserSelector.exe

# Альтернативно використовувати timestamp сервер:
signtool sign /f certificate.pfx /p password /tr http://timestamp.digicert.com /td sha256 /fd sha256 BrowserSelector.exe
```

### 12. Критерії успіху

Проєкт завершений коли:
1. ✅ Один .exe файл (статично слінкований)
2. ✅ Конфігурація в JSON поряд з exe
3. ✅ Вибір браузера клавіатурою (стрілки + Enter)
4. ✅ Tab переключає між ListBox та кнопкою Settings
5. ✅ Вікно налаштувань дозволяє керувати командами
6. ✅ Команда за замовчуванням обрана при запуску
7. ✅ URL коректно передається в браузер
8. ✅ Працює з NVDA та Windows Narrator
9. ✅ Обробка помилок (відсутні файли, невірні шляхи)
10. ✅ Реєстрація як браузер Windows
11. ✅ Відсутнє вікно консолі
12. ✅ Малий розмір (~500KB-1MB зі статичним лінкуванням)

## Додаткові ресурси

### Документація
- Win32 API: https://learn.microsoft.com/en-us/windows/win32/api/
- GCC Manual: https://gcc.gnu.org/onlinedocs/
- cJSON: https://github.com/DaveGamble/cJSON
- MinGW-w64: https://www.mingw-w64.org/

### Корисні інструменти
- Resource Hacker: Перегляд/редагування ресурсів .exe
- Dependency Walker: Перевірка залежностей DLL
- Process Monitor: Моніторинг файлових операцій
- UPX: Стиснення виконуваних файлів

#### 12.1 Windows-специфічні інструменти
```cmd
# Перевірка залежностей .exe
dumpbin /dependents bin\BrowserSelector.exe

# Аналізатор продуктивності
# Win+R -> perfmon

# Моніторинг реєстру Windows
# Win+R -> regedit

# Перевірка цифрового підпису
signtool verify /pa bin\BrowserSelector.exe
```

#### 12.2 Інтеграція з Windows

#### 12.2.1 Реєстрація як браузер за замовчуванням
```reg
Windows Registry Editor Version 5.00

[HKEY_CLASSES_ROOT\BrowserSelector]
@="URL:BrowserSelector Protocol"
"URL Protocol"=""

[HKEY_CLASSES_ROOT\BrowserSelector\shell]

[HKEY_CLASSES_ROOT\BrowserSelector\shell\open]

[HKEY_CLASSES_ROOT\BrowserSelector\shell\open\command]
@="\"C:\\Program Files\\BrowserSelector\\BrowserSelector.exe\" \"%1\""
```

#### 12.2.2 Додавання до контекстного меню
```reg
Windows Registry Editor Version 5.00

[HKEY_CLASSES_ROOT\http\shell\BrowserSelector]
@="Open with Browser Selector"

[HKEY_CLASSES_ROOT\http\shell\BrowserSelector\command]
@="\"C:\\Program Files\\BrowserSelector\\BrowserSelector.exe\" \"%1\""

[HKEY_CLASSES_ROOT\https\shell\BrowserSelector]
@="Open with Browser Selector"

[HKEY_CLASSES_ROOT\https\shell\BrowserSelector\command]
@="\"C:\\Program Files\\BrowserSelector\\BrowserSelector.exe\" \"%1\""
```