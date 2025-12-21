#include "i18n.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Module instance handle
static HINSTANCE g_hInstance = NULL;
static LanguageId g_currentLanguage = LANG_ID_ENGLISH;

// Internal buffers for string retrieval
static wchar_t g_wideBuffer[1024];
static char g_ansiBuffer[2048];

// Language names for debugging
static const char* g_languageNames[LANG_COUNT] = {
    "English",
    "Ukrainian",
    "German",
    "French",
    "Spanish",
    "Italian",
    "Polish",
    "Dutch",
    "Portuguese"
};

// Map Windows LANGID to our language enum
static LanguageId MapWindowsLanguage(LANGID langId) {
    WORD primaryLang = PRIMARYLANGID(langId);
    
    switch (primaryLang) {
        case LANG_UKRAINIAN:
            return LANG_ID_UKRAINIAN;
        case LANG_GERMAN:
            return LANG_ID_GERMAN;
        case LANG_FRENCH:
            return LANG_ID_FRENCH;
        case LANG_SPANISH:
            return LANG_ID_SPANISH;
        case LANG_ITALIAN:
            return LANG_ID_ITALIAN;
        case LANG_POLISH:
            return LANG_ID_POLISH;
        case LANG_DUTCH:
            return LANG_ID_DUTCH;
        case LANG_PORTUGUESE:
            return LANG_ID_PORTUGUESE;
        case LANG_ENGLISH:
        default:
            return LANG_ID_ENGLISH;
    }
}

void I18n_Init(HINSTANCE hInstance) {
    g_hInstance = hInstance;
    
    // Get Windows UI language
    LANGID langId = GetUserDefaultUILanguage();
    g_currentLanguage = MapWindowsLanguage(langId);
    
    // Verify that we can load strings for this language
    // Try to load a test string - if it fails, fall back to English
    if (g_currentLanguage != LANG_ID_ENGLISH) {
        // Calculate resource language
        WORD resLang;
        switch (g_currentLanguage) {
            case LANG_ID_UKRAINIAN:
                resLang = MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT);
                break;
            case LANG_ID_GERMAN:
                resLang = MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT);
                break;
            case LANG_ID_FRENCH:
                resLang = MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT);
                break;
            case LANG_ID_SPANISH:
                resLang = MAKELANGID(LANG_SPANISH, SUBLANG_DEFAULT);
                break;
            case LANG_ID_ITALIAN:
                resLang = MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT);
                break;
            case LANG_ID_POLISH:
                resLang = MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT);
                break;
            case LANG_ID_DUTCH:
                resLang = MAKELANGID(LANG_DUTCH, SUBLANG_DEFAULT);
                break;
            case LANG_ID_PORTUGUESE:
                resLang = MAKELANGID(LANG_PORTUGUESE, SUBLANG_DEFAULT);
                break;
            default:
                resLang = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
                break;
        }
        
        // Try to load with specific language
        // If it fails, Windows will fall back automatically through resource lookup
        SetThreadUILanguage(resLang);
    }
}

const wchar_t* I18n_GetStringW(UINT stringId) {
    // Clear buffer
    g_wideBuffer[0] = L'\0';
    
    // Load string from resources
    int len = LoadStringW(g_hInstance, stringId, g_wideBuffer, 
                          sizeof(g_wideBuffer) / sizeof(wchar_t));
    
    if (len <= 0) {
        // String not found - return empty string
        g_wideBuffer[0] = L'\0';
    }
    
    return g_wideBuffer;
}

const char* I18n_GetStringA(UINT stringId) {
    // First get wide string
    const wchar_t* wideStr = I18n_GetStringW(stringId);
    
    // Convert to ANSI
    int len = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, 
                                   g_ansiBuffer, sizeof(g_ansiBuffer), 
                                   NULL, NULL);
    
    if (len <= 0) {
        g_ansiBuffer[0] = '\0';
    }
    
    return g_ansiBuffer;
}

LanguageId I18n_GetCurrentLanguage(void) {
    return g_currentLanguage;
}

const char* I18n_GetLanguageName(LanguageId langId) {
    if (langId >= 0 && langId < LANG_COUNT) {
        return g_languageNames[langId];
    }
    return "Unknown";
}
