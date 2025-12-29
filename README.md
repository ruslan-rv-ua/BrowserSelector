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
A lightweight, accessible Windows application for selecting which browser opens your links
</p>

## Overview

Browser Selector is a portable Windows desktop application that intercepts URL opening requests and presents you with a menu to choose which browser or command to use. It's designed to be your default browser, giving you control over how links are opened.

## Key Features

- **Set as Default Browser**: Register as Windows default browser to intercept all link clicks
- **Accessibility First**: Full support for NVDA and Windows Narrator screen readers
- **Multi-Language Interface**: Automatic UI language detection supporting 9 languages
- **Auto-Open Timer**: Configurable countdown (1-10 seconds) to automatically open your preferred browser
- **Shell Command Support**: Execute any shell command with URL parameter, including pipes and redirects
- **Keyboard-Driven**: Navigate with arrow keys, Enter, Escape, Tab, and numbers 1-9
- **Portable**: Single executable with JSON configuration file

## Supported Languages

- English
- Ukrainian
- German
- French
- Spanish
- Italian
- Polish
- Dutch
- Portuguese

The application automatically detects your Windows UI language and displays the interface accordingly. If your language is not supported, it defaults to English.

## Installation

### Using Scoop (Recommended)

```cmd
scoop bucket add ruslan-rv-ua https://github.com/ruslan-rv-ua/scoop-bucket
scoop install browserselector
```

### Manual Installation

1. Download the latest `BrowserSelector-x.x.x-win64.zip` from the [Releases](https://github.com/ruslan-rv-ua/BrowserSelector/releases) page
2. Extract to your preferred location
3. Run `BrowserSelector.exe`

### Building from Source

**Prerequisites:**
- MinGW-w64 GCC 8.1.0 or newer
- Make (optional)

**Using Make:**
```cmd
make              # Build debug version
make release      # Build optimized release version
make install      # Build and copy config to bin/
make clean        # Remove build artifacts
```

**Manual build:**
```cmd
# Create directories
mkdir obj bin

# Compile sources
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/main.c -o obj/main.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/config/config.c -o obj/config.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/ui/mainwindow.c -o obj/mainwindow.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/ui/settings.c -o obj/settings.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/ui/commandeditor.c -o obj/commandeditor.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/executor/executor.c -o obj/executor.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/registry/registry.c -o obj/registry.o
gcc -std=c11 -Wall -O2 -Iinclude -DUNICODE -D_UNICODE -c src/i18n/i18n.c -o obj/i18n.o
gcc -std=c11 -Wall -O2 -Iinclude -c include/cJSON.c -o obj/cJSON.o

# Compile resources
windres --codepage=65001 -Isrc resources/app.rc -O coff -o obj/app.res

# Link
gcc -mwindows -static-libgcc -static -o bin/BrowserSelector.exe ^
    obj/main.o obj/config.o obj/mainwindow.o obj/settings.o ^
    obj/commandeditor.o obj/executor.o obj/registry.o obj/i18n.o obj/cJSON.o obj/app.res ^
    -lcomctl32 -ladvapi32 -lshell32 -luser32 -lgdi32 -lkernel32 -lcomdlg32 -lole32
```

## Usage

### Setting as Default Browser

1. Run `BrowserSelector.exe`
2. Click the "Set as default browser" button
3. In the Windows Settings window that opens:
   - Scroll to "Web browser"
   - Click on the current browser name
   - Select "BrowserSelector" from the list
4. Close Settings and click "Yes" to confirm

The button will change to "Unregister as default" when Browser Selector is set as your default browser.

### Command Line Usage

```cmd
# Launch with URL
BrowserSelector.exe "https://example.com"

# Launch with local file
BrowserSelector.exe "file:///C:/path/to/file.html"

# Launch without URL (for testing)
BrowserSelector.exe
```

### Keyboard Shortcuts

- **Arrow Up/Down**: Navigate browser list
- **Enter**: Open URL with selected browser
- **1-9**: Quick select browser by number
- **Tab**: Cycle focus between list and buttons
- **Escape**: Close without opening
- **Any Key**: Cancel auto-open timer

### Auto-Open Timer

When a URL is opened, Browser Selector displays a countdown timer that automatically launches the default browser after the configured wait time (default: 10 seconds). Any user interaction cancels the timer.

**Configuring wait time:**
1. Open Settings
2. Adjust "Wait Time (1-10 sec)" field
3. Changes are saved automatically

## Configuration

The `config.json` file is created in the same directory as `BrowserSelector.exe` on first run. You can edit it manually or use the Settings interface.

### Configuration Structure

```json
{
  "settings": {
    "defaultCommandIndex": 0,
    "waitTime": 10
  },
  "commands": [
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
    }
  ]
}
```

### Configuration Fields

- **defaultCommandIndex**: Index (0-based) of the default browser to auto-open
- **waitTime**: Auto-open countdown duration in seconds (1-10)
- **name**: Display name in the selector
- **command**: Command to execute. Use `{url}` as placeholder for the URL

### Command Format

Commands are executed via `cmd.exe /c`, which enables:
- Shell commands and pipes: `echo {url} | clip`
- Built-in commands: `start {url}`
- Executable paths (absolute or in PATH): `chrome.exe --incognito {url}`
- Complex shell operations: `powershell -Command "Write-Output {url}"`

### Managing Configuration

**Via Settings Interface:**
1. Click "Settings" button
2. Use Add/Edit/Delete buttons to manage commands
3. Use "Move Up"/"Move Down" to reorder
4. Click "Set Default" to change the auto-open browser
5. Changes are saved automatically

**Manual Editing:**
1. Close Browser Selector if running
2. Edit `config.json` in a text editor
3. Save and restart Browser Selector

## Technical Details

### System Requirements

- Windows 10/11 (x64)
- No additional runtime libraries required (statically compiled)
- Approximately 500KB-1MB disk space

### Architecture

- **Language**: C11
- **Compiler**: GCC (MinGW-w64)
- **GUI**: Win32 API with native controls
- **Configuration**: JSON (cJSON library)
- **Localization**: Windows String Resources (STRINGTABLE)
- **Build System**: Makefile

## Troubleshooting

### Configuration Issues

**Corrupted config.json:**
If the configuration file becomes corrupted, Browser Selector will detect this on startup and offer to create a new default configuration.

**Manually reset configuration:**
Delete `config.json` from the application directory. A new default configuration will be created automatically on next launch.

### Command Execution Issues

**"Command not found" error:**
- Verify the command is in your system PATH
- Use absolute paths: `C:\Program Files\Browser\browser.exe {url}`
- Check spelling and executable name

**Command doesn't open:**
- Ensure `{url}` placeholder is included if the command needs the URL
- Test the command in `cmd.exe` manually
- Check that the browser/application is installed

### Registration Issues

**Cannot set as default browser:**
- Try running as Administrator
- Ensure no antivirus is blocking Registry modifications
- Manually open Windows Settings and select BrowserSelector

**Button shows wrong state:**
The button text automatically updates based on actual registration status. If it appears incorrect, try clicking it to refresh the state.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

- JSON parsing: [cJSON](https://github.com/DaveGamble/cJSON) by Dave Gamble
- Inspired by the need for flexible and accessible browser selection on Windows

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and release notes.