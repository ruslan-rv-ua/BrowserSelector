<p align="center">
  <img src="resources/logo.png" alt="Browser Selector Logo" width="200"/>
</p>

<h1 align="center">Browser Selector</h1>

<p align="center">
  <strong>Browser Selector</strong> is a <strong>accessible</strong> portable Windows application that allows you to easily choose which browser to open a link with. When you open a URL, the application shows a menu with configured browsers/commands for you to choose from.
</p>

## 📱 Key Features

- **👁️ Accessibility first!**: Works with NVDA and Windows Narrator
- **🌍 Multi-language**: Automatic UI language detection (English, Ukrainian, German, French, Spanish, Italian, Polish, Dutch, Portuguese)
- **🖥️ Clean Interface**
- **📦 Portable**: Single .exe file with JSON configuration
- **⌨️ Quick Navigation**: Use arrow keys, Enter, Escape, and numbers 1-9 for quick selection
- **🔄 Auto-Open**: Default browser opens automatically after configurable time (1-10 seconds)
- **🎛️ Flexible Configuration**: Add, edit, and reorder browsers through the settings window

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

- **↑/↓** - Navigate through browser list
- **Enter** - Open URL with selected browser
- **1-9** - Quick select browser by number
- **Escape** - Close without action
- **Tab** - Switch between list and buttons
- **Any Key** - Cancel auto-open timer

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
  "name": "Chrome Incognito",
  "command": "chrome.exe",
  "arguments": "--incognito {url}"
},
{
  "name": "Firefox Private",
  "command": "firefox.exe",
  "arguments": "-private-window {url}"
},
{
  "name": "Edge with specific profile",
  "command": "msedge.exe",
  "arguments": "--profile-directory=\"Default\" {url}"
}
```

### Browser Path Types

- **Program name**: `"chrome.exe"` (if in PATH)
- **Absolute path**: `"C:\\Program Files\\Mozilla Firefox\\firefox.exe"`
- **Relative path**: `"..\\browsers\\custom.exe"`

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

Windows 10/11 requires users to manually confirm default browser changes for security reasons. This applies to ALL browsers, including Firefox, Chrome, and Edge when first installed.

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

### Configuration File Format

The `config.json` file is stored in the same directory as the executable:

```json
{
  "settings": {
    "defaultCommandIndex": 0,
    "waitTime": 10
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

#### Field Descriptions

- **name**: Display name in the selector
- **command**: Path to executable (can be absolute, relative, or just the exe name if in PATH)
- **arguments**: Command line arguments. Use `{url}` as placeholder for the URL
- **defaultCommandIndex**: Index of the default browser (0-based)
- **waitTime**: Auto-open timer duration in seconds (1-10, default: 10)

## 📄 License

MIT License

## 👏 Credits

- [cJSON](https://github.com/DaveGamble/cJSON) - JSON parsing library

## 🚀 Release Creation (git-flow)

Short steps for creating a release using `git-flow` on Windows (CMD / Git Bash / MSYS2).

1) Start the release branch (example for version `v1.0.0`):

```bash
git flow release start v1.0.0
```

2) Bump the version in `resources/app.rc` (`FILEVERSION` / `PRODUCTVERSION`) and update `CHANGELOG.md` if needed, then commit:

```bash
# Edit resources/app.rc and CHANGELOG.md
git add resources/app.rc CHANGELOG.md
git commit -m "Bump version to v1.0.0"
```

3) Build the release (run inside MSYS2 MINGW64 or an environment with `make` and MinGW available):

```bash
make release
# Output: bin/BrowserSelector.exe
```

4) Test the generated executable:

Windows CMD:
```cmd
bin\BrowserSelector.exe "https://example.com"
```

MSYS2 / Bash:
```bash
./bin/BrowserSelector.exe "https://example.com"
```

5) (Optional) Sign the executable if you have a certificate:

```cmd
signtool sign /f cert.pfx /p <password> /tr http://timestamp.digicert.com /td sha256 /fd sha256 bin\BrowserSelector.exe
```

6) Finish the release (creates a tag, merges into `master` and `develop`) and push:

```bash
git flow release finish v1.0.0
git push origin develop
git push origin master
git push --tags
```

7) Create a GitHub Release with tag `v1.0.0` and attach `bin/BrowserSelector.exe` as an artifact.
