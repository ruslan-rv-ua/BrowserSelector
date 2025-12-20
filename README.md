# Browser Selector

A portable Windows desktop application that acts as a browser selector. When a URL is opened, the application shows a menu with configured browsers/commands for the user to choose from.

## Features

- **Portable**: Single .exe file with JSON configuration
- **Keyboard Navigation**: Use arrow keys, Enter, Escape, and number keys (1-9) for quick selection
- **Configurable**: Add, edit, delete, and reorder browsers through the Settings window
- **Screen Reader Compatible**: Works with NVDA and Windows Narrator
- **No Console Window**: Clean GUI-only experience
- **Windows Registry Integration**: Can register as a browser handler

## Building

### Prerequisites

1. **MinGW-w64 GCC** (8.1.0 or newer)
   - Install via MSYS2: `pacman -S mingw-w64-x86_64-gcc`
   - Or download from [winlibs.com](https://winlibs.com/)

2. **Make** (optional, for Makefile usage)
   - Install via MSYS2: `pacman -S mingw-w64-x86_64-make`

3. Add MinGW bin directory to PATH (e.g., `C:\msys64\mingw64\bin`)

### Build Commands

```cmd
:: Build the project
make

:: Build release version (optimized, stripped)
make release

:: Build and copy config.json to bin
make install

:: Clean build files
make clean

:: Run the application
make run
```

### Manual Build (without Make)

```cmd
:: Create directories
mkdir obj
mkdir bin

:: Compile source files
gcc -std=c11 -Wall -O2 -Iinclude -c src/main.c -o obj/main.o
gcc -std=c11 -Wall -O2 -Iinclude -c src/config/config.c -o obj/config.o
gcc -std=c11 -Wall -O2 -Iinclude -c src/ui/mainwindow.c -o obj/mainwindow.o
gcc -std=c11 -Wall -O2 -Iinclude -c src/ui/settings.c -o obj/settings.o
gcc -std=c11 -Wall -O2 -Iinclude -c src/ui/commandeditor.c -o obj/commandeditor.o
gcc -std=c11 -Wall -O2 -Iinclude -c src/executor/executor.c -o obj/executor.o
gcc -std=c11 -Wall -O2 -Iinclude -c src/registry/registry.c -o obj/registry.o
gcc -std=c11 -Wall -O2 -Iinclude -c include/cJSON.c -o obj/cJSON.o

:: Compile resources
windres -i resources/app.rc -o obj/app.res

:: Link
gcc -mwindows -static-libgcc -static -o bin/BrowserSelector.exe ^
    obj/main.o obj/config.o obj/mainwindow.o obj/settings.o ^
    obj/commandeditor.o obj/executor.o obj/registry.o obj/cJSON.o obj/app.res ^
    -lcomctl32 -ladvapi32 -lshell32 -luser32 -lgdi32 -lkernel32 -lcomdlg32 -lole32

:: Copy config
copy config.json bin\
```

## Usage

### Basic Usage

```cmd
:: Open without URL
BrowserSelector.exe

:: Open with URL
BrowserSelector.exe "https://example.com"
```

### Keyboard Shortcuts

- **↑/↓** - Navigate through browser list
- **Enter** - Open URL with selected browser
- **1-9** - Quick select browser by number
- **Escape** - Close without action
- **Tab** - Switch between list and Settings button

### Windows Registration

```cmd
:: Register as browser handler
BrowserSelector.exe register

:: Unregister
BrowserSelector.exe unregister
```

After registration, you can set Browser Selector as your default browser in Windows Settings.

## Configuration

The `config.json` file is stored in the same directory as the executable.

```json
{
  "settings": {
    "defaultCommandIndex": 0
  },
  "commands": [
    {
      "name": "Chrome",
      "command": "chrome.exe",
      "arguments": "{url}"
    },
    {
      "name": "Firefox",
      "command": "C:\\Program Files\\Mozilla Firefox\\firefox.exe",
      "arguments": "{url}"
    }
  ]
}
```

### Command Configuration

- **name**: Display name in the selector
- **command**: Path to executable (can be absolute, relative, or just the exe name if in PATH)
- **arguments**: Command line arguments. Use `{url}` as placeholder for the URL

### Examples

```json
{
  "name": "Chrome Incognito",
  "command": "chrome.exe",
  "arguments": "--incognito {url}"
},
{
  "name": "Firefox Private",
  "command": "firefox.exe",
  "arguments": "-private-window {url}"
}
```

## Project Structure

```
browserselector/
├── src/
│   ├── main.c              # Entry point
│   ├── config/
│   │   ├── config.c        # Configuration handling
│   │   └── config.h
│   ├── ui/
│   │   ├── mainwindow.c    # Main window
│   │   ├── mainwindow.h
│   │   ├── settings.c      # Settings window
│   │   ├── settings.h
│   │   ├── commandeditor.c # Add/Edit dialog
│   │   └── commandeditor.h
│   ├── executor/
│   │   ├── executor.c      # Command execution
│   │   └── executor.h
│   └── registry/
│       ├── registry.c      # Windows registry
│       └── registry.h
├── include/
│   ├── cJSON.h             # cJSON library
│   └── cJSON.c
├── resources/
│   ├── app.rc              # Resource file
│   └── manifest.xml        # Application manifest
├── Makefile
├── config.json
└── README.md
```

## License

MIT License

## Credits

- [cJSON](https://github.com/DaveGamble/cJSON) - JSON parsing library
