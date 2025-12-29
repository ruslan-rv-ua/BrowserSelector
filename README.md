<p align="center">
  <img src="resources/logo.png" alt="Browser Selector Logo" width="200"/>
</p>

<h1 align="center">Browser Selector</h1>

<p align="center">
  <img src="https://img.shields.io/github/v/release/ruslan-rv-ua/BrowserSelector" alt="Version">
  <img src="https://img.shields.io/github/license/ruslan-rv-ua/BrowserSelector" alt="License">
  <img src="https://img.shields.io/badge/platform-Windows-blue" alt="Platform">
</p>

<p align="center">
  <strong>Browser Selector</strong> is <strong>an accessible</strong>, portable Windows application that allows you to easily choose which browser to open a link with. When you open a URL, the application presents a menu with your configured browsers for you to choose from.
</p>

## 📱 Key Features

- **👁️ Accessibility first!**: Works with NVDA and Windows Narrator
- **🌍 Multi-language**: Automatic UI language detection (English, Ukrainian, German, French, Spanish, Italian, Polish, Dutch, Portuguese)
- **🖥️ Clean Interface**
- **📦 Portable**: Single .exe file with JSON configuration
- **⌨️ Quick Navigation**: Use Arrow keys, Enter, Escape, and numbers 1-9 for quick selection
- **🔄 Auto-Open**: Default browser opens automatically after a configurable countdown (1-10 seconds)
- **🎛️ Flexible Configuration**: Add, edit, and reorder browsers visually through the Settings window

## 🚀 Quick Start

1. **Download** `BrowserSelector.exe` from the [Releases](https://github.com/ruslan-rv-ua/BrowserSelector/releases) page
2. **Run** `BrowserSelector.exe "https://example.com"` or just `BrowserSelector.exe`
3. **Select** a browser from the list or wait for automatic opening (10 seconds by default)

### How to Set as Default Browser

1. Click the **"Set as Default"** button
2. In opened Windows Settings window select **BrowserSelector** as the default web browser
3. Confirm the changes by clicking "Yes" in Browser Selector
4. Now all links will open through Browser Selector!

## 🎮 How to Use

### Basic Commands

```cmd
:: Open without URL
BrowserSelector.exe

:: Open with URL
BrowserSelector.exe "https://example.com"

:: Open with local file
BrowserSelector.exe "file:///C:/Users/Test/Desktop/index.html"
```

### Keyboard Shortcuts

- **↑/↓** - Navigate through the browser list
- **Enter** - Open URL with the selected browser
- **1-9** - Select browser by number
- **Escape** - Close Browser Selector without opening any link
- **Tab** - Switch focus between list and buttons
- **Any Key** - Pause/Cancel the auto-open timer

### Auto-Open Timer

The application features an auto-open timer that automatically launches the default browser after a configured wait time:

- **Default time**: 10 seconds
- **Configurable range**: 1-10 seconds
- **Visual countdown**: Shows remaining seconds in the main window
- **Cancel on interaction**: Any user action cancels the timer
- **Settings**: Configure wait time in the settings window

## 🌍 Supported Languages

Browser Selector automatically detects your Windows UI language and displays the interface accordingly:

| Language | Code |
|----------|------|
| English | en (default) |
| Ukrainian | uk |
| German | de |
| French | fr |
| Spanish | es |
| Italian | it |
| Polish | pl |
| Dutch | nl |
| Portuguese | pt |

If your Windows language is not in this list, the application falls back to English.

## ⚙️ Browser Configuration

### Settings Window

Click the **"Settings"** button to access configuration:

- **Add browsers**: Click "Add" to add a new browser
- **Edit**: Select a browser and click "Edit"
- **Delete**: Select a browser and click "Delete"
- **Reorder**: Use "Up"/"Down" to change order
- **Auto-open time**: Configure `Wait Time` (1-10 seconds)

### Configuration Examples

```json
{
  "name": "Copy to Clipboard",
  "command": "echo {url} | clip"
},
{
  "name": "Chrome Incognito",
  "command": "chrome.exe --incognito {url}"
},
{
  "name": "Firefox Profile",
  "command": "firefox.exe -P default {url}"
},
{
  "name": "Helium",
  "command": "helium.exe {url}"
},
{
  "name": "Firefox Private",
  "command": "firefox.exe -private-window {url}"
},
{
  "name": "Edge with specific profile",
  "command": "msedge.exe --profile-directory=\"Default\" {url}"
}
```

### Command Format

The `command` field contains the full command to execute, including any arguments. Use `{url}` as a placeholder for the URL that will be opened.

## 🔧 Default Browser Registration

### Registration Process

1. Click the **"Set as Default"** button
2. Windows Settings will open automatically
3. Scroll to the **Web browser** section
4. Click on the current browser name
5. Select **BrowserSelector** from the list
6. Close Settings and return to the app
7. Click "Yes" to confirm the changes

### Button States

- **"Set as Default"** - when the app is not registered or not set as default browser
- **"Unregister"** - when already set as default browser

### Why Manual Confirmation?

Windows 10 and 11 require users to manually confirm changes to the default browser for security reasons. This applies to **all** browsers, including Firefox, Chrome, and Edge. Only you, the user, can authorize this change.

---

## 🛠️ Technical Documentation

### System Requirements

- **Operating System**: Windows 10/11 (x64)
- **Runtime**: No additional libraries required (statically compiled)
- **Space**: ~500KB-1MB on disk

### Building from Source

#### Prerequisites

1. **MinGW-w64 GCC** (8.1.0 or newer)
   - Install via MSYS2: `pacman -S mingw-w64-x86_64-gcc`
   - Or download from [winlibs.com](https://winlibs.com/)

2. **Make** (optional, for Makefile usage)
   - Install via MSYS2: `pacman -S mingw-w64-x86_64-make`

3. Add MinGW bin directory to PATH (e.g., `C:\msys64\mingw64\bin`)

#### Build Commands

```cmd
:: Build the project
make

:: Build release version (optimized)
make release

:: Build and copy config.json to bin
make install

:: Clean build files
make clean

:: Build and run
make run
```

#### Manual Build (without Make)

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

:: Copy configuration
copy config.json bin\
```

### Project Structure

- `src/` - Source code
  - `ui/` - Interface logic (GDI/WinAPI)
  - `config/` - JSON configuration handling
  - `registry/` - Windows Registry operations
  - `i18n/` - Internationalization helper functions
- `resources/` - Icons, localized strings (`strings.rc`)
- `include/` - Header files and external libraries (cJSON)

### Adding Translations

Localization is handled via Windows Resource files which allows the app to automatically adapt to the user's OS language. To add a new language:

1. Open `resources/strings.rc`.
2. Add a new `STRINGTABLE` block with the appropriate `LANGUAGE` identifier.
3. Translate the string definitions.
4. Rebuild the project.

### Configuration File Format

The `config.json` file is stored in the same directory as the executable:

```json
{
  "settings": {
    "defaultCommandIndex": 1,
    "waitTime": 10
  },
  "commands": [
    {
      "name": "Copy to Clipboard",
      "command": "echo {url} | clip"
    },
    {
      "name": "Chrome",
      "command": "chrome.exe {url}"
    },
    {
      "name": "Firefox",
      "command": "C:\\Program Files\\Mozilla Firefox\\firefox.exe {url}"
    }
  ]
}
```

#### Field Descriptions

- **name**: Display name in the selector
- **command**: Full command to execute. Use `{url}` as placeholder for the URL. Can include shell commands, pipes, and arguments
- **defaultCommandIndex**: Index of the default browser (0-based)
- **waitTime**: Auto-open timer duration in seconds (1-10, default: 10)

## ❓ Troubleshooting

### Configuration file is corrupted
If `config.json` becomes invalid or corrupted, the application will detect this on startup. It will attempt to create a new default configuration file. You can also manually delete `config.json` to force a reset.

### Browser not opening
- Check if the command in `config.json` is correct.
- If using just the executable name (e.g., `chrome.exe`), ensure it is available in your system's PATH.
- Verify that the `{url}` placeholder is present in the command if needed.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

MIT License

## 👏 Credits

- [cJSON](https://github.com/DaveGamble/cJSON) - JSON parsing library

