# Change Log

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.1] - 2024-12-22

### Added
- Accessibility features including keyboard navigation and screen reader support
- Auto-open timer with countdown display functionality
- Comprehensive i18n support for English, Ukrainian, German, French, Spanish, Italian, Polish, Dutch, Portuguese

### Changed
- Improved documentation clarity
- Added project logo to README

### Fixed
- Registry warning by casting GetProcAddress return value to uintptr_t
- Focus handling after window activation

## [0.1.0] - 2024-12-20

### Added
- Initial implementation of browser selector application with Windows GUI

---

## Development Notes

### Key Features Implemented
- **Accessibility**: Full keyboard navigation, screen reader support, Tab focus handling
- **Internationalization**: Support for 9 languages (English, Ukrainian, German, French, Spanish, Italian, Polish, Dutch, Portuguese)
- **Auto-open Timer**: Configurable wait time (1-10 seconds) with countdown display
- **Smart Registration**: Automatic detection of current default browser status
- **Security**: Safe string operations, proper memory management
- **Portability**: Single executable file with embedded resources
