#ifndef I18N_H
#define I18N_H

#include <windows.h>

// String IDs - organized by category
// Range 100-199: Main window strings
#define IDS_APP_TITLE               100
#define IDS_SETTINGS_BTN            101
#define IDS_SET_DEFAULT_BTN         102
#define IDS_UNREGISTER_BTN          103
#define IDS_COUNTDOWN_SECONDS       104
#define IDS_COUNTDOWN_SECOND        105
#define IDS_COUNTDOWN_NOW           106

// Range 200-299: Settings window strings
#define IDS_SETTINGS_TITLE          200
#define IDS_ADD_BTN                 201
#define IDS_EDIT_BTN                202
#define IDS_DELETE_BTN              203
#define IDS_MOVE_UP_BTN             204
#define IDS_MOVE_DOWN_BTN           205
#define IDS_SET_DEFAULT_CMD_BTN     206
#define IDS_CLOSE_BTN               207
#define IDS_WAITTIME_LABEL          208
#define IDS_DEFAULT_MARKER          209
#define IDS_MAX_COMMANDS_WARNING    210
#define IDS_CONFIRM_DELETE_TITLE    211
#define IDS_CONFIRM_DELETE_MSG      212

// Range 300-399: Command editor strings
#define IDS_ADD_BROWSER_TITLE       300
#define IDS_EDIT_BROWSER_TITLE      301
#define IDS_NAME_LABEL              302
#define IDS_COMMAND_LABEL           303
#define IDS_ARGUMENTS_LABEL         304
#define IDS_URL_PLACEHOLDER_HINT    305
#define IDS_OK_BTN                  306
#define IDS_CANCEL_BTN              307
#define IDS_BROWSE_TITLE            308
#define IDS_BROWSE_FILTER_EXE       309
#define IDS_BROWSE_FILTER_ALL       310
#define IDS_VALIDATION_NAME_EMPTY   311
#define IDS_VALIDATION_CMD_EMPTY    312
#define IDS_VALIDATION_ERROR_TITLE  313

// Range 400-499: Error messages
#define IDS_ERROR                   400
#define IDS_WARNING                 401
#define IDS_SUCCESS                 402
#define IDS_CMD_NOT_FOUND           403
#define IDS_CMD_NOT_FOUND_HINT      404
#define IDS_LAUNCH_FAILED           405
#define IDS_MEMORY_ERROR            406

// Range 500-599: Registration messages
#define IDS_REG_SUCCESS_TITLE       500
#define IDS_REG_SUCCESS_MSG         501
#define IDS_REG_FAILED_TITLE        502
#define IDS_REG_FAILED_MSG          503
#define IDS_UNREG_SUCCESS_TITLE     504
#define IDS_UNREG_SUCCESS_MSG       505
#define IDS_UNREG_FAILED_TITLE      506
#define IDS_UNREG_FAILED_MSG        507
#define IDS_SET_DEFAULT_TITLE       508
#define IDS_SET_DEFAULT_MSG         509
#define IDS_VERIFY_TITLE            510
#define IDS_VERIFY_MSG              511
#define IDS_NOW_DEFAULT_MSG         512
#define IDS_NOT_DEFAULT_MSG         513

// Range 600-699: Main application messages
#define IDS_CONFIG_LOAD_FAILED      600
#define IDS_CONFIG_LOAD_FAILED_MSG  601
#define IDS_CONFIG_CREATE_FAILED    602
#define IDS_CONFIG_LOAD_RETRY_FAIL  603
#define IDS_NO_BROWSERS_TITLE       604
#define IDS_NO_BROWSERS_MSG         605
#define IDS_WINDOW_CREATE_FAILED    606
#define IDS_CONFIG_CORRUPTED_MSG    607
#define IDS_CONFIG_CREATE_BTN       608
#define IDS_CONFIG_EXIT_BTN         609

// Supported languages
typedef enum {
    LANG_ID_ENGLISH = 0,
    LANG_ID_UKRAINIAN,
    LANG_ID_GERMAN,
    LANG_ID_FRENCH,
    LANG_ID_SPANISH,
    LANG_ID_ITALIAN,
    LANG_ID_POLISH,
    LANG_ID_DUTCH,
    LANG_ID_PORTUGUESE,
    LANG_COUNT
} LanguageId;

// Initialize i18n system - must be called at application start
void I18n_Init(HINSTANCE hInstance);

// Get localized string by ID
// Returns pointer to internal buffer - valid until next GetString call
const wchar_t* I18n_GetStringW(UINT stringId);

// Get localized string as ANSI (UTF-8 converted to current code page)
// Returns pointer to internal buffer - valid until next GetString call
const char* I18n_GetStringA(UINT stringId);

// Get current language ID
LanguageId I18n_GetCurrentLanguage(void);

// Get language name (for debugging)
const char* I18n_GetLanguageName(LanguageId langId);

#endif // I18N_H
