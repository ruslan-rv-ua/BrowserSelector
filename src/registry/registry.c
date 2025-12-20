#include "registry.h"
#include <stdio.h>
#include <string.h>
#include <shlobj.h>
#include <shobjidl.h>

#define APP_NAME "BrowserSelector"
#define APP_DESC "Browser Selector"

static BOOL SetRegistryValue(HKEY hKey, const char* subKey, const char* valueName, const char* value) {
    HKEY hSubKey;
    LONG result = RegCreateKeyExA(hKey, subKey, 0, NULL, 
                                   REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL);
    if (result != ERROR_SUCCESS) {
        return FALSE;
    }
    
    result = RegSetValueExA(hSubKey, valueName, 0, REG_SZ, 
                           (const BYTE*)value, (DWORD)(strlen(value) + 1));
    RegCloseKey(hSubKey);
    
    return result == ERROR_SUCCESS;
}

static BOOL DeleteRegistryKey(HKEY hKey, const char* subKey) {
    return RegDeleteTreeA(hKey, subKey) == ERROR_SUCCESS;
}

BOOL RegisterAsBrowser(const char* exePath) {
    char command[MAX_PATH + 10];
    snprintf(command, sizeof(command), "\"%s\" \"%%1\"", exePath);
    
    // Register URL protocol handler
    // HKEY_CURRENT_USER\Software\Classes\BrowserSelector
    char keyPath[256];
    
    // Create main key
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\%s", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, NULL, "URL:" APP_DESC)) {
        return FALSE;
    }
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, "URL Protocol", "")) {
        return FALSE;
    }
    
    // Create shell\open\command
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\%s\\shell\\open\\command", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, NULL, command)) {
        return FALSE;
    }
    
    // Register for http protocol
    if (!SetRegistryValue(HKEY_CURRENT_USER, "Software\\Classes\\http\\shell\\" APP_NAME, NULL, "Open with " APP_DESC)) {
        return FALSE;
    }
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\http\\shell\\%s\\command", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, NULL, command)) {
        return FALSE;
    }
    
    // Register for https protocol
    if (!SetRegistryValue(HKEY_CURRENT_USER, "Software\\Classes\\https\\shell\\" APP_NAME, NULL, "Open with " APP_DESC)) {
        return FALSE;
    }
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\https\\shell\\%s\\command", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, NULL, command)) {
        return FALSE;
    }
    
    // Register application in Registered Applications
    if (!SetRegistryValue(HKEY_CURRENT_USER, 
                         "Software\\RegisteredApplications", 
                         APP_NAME, 
                         "Software\\Clients\\StartMenuInternet\\" APP_NAME "\\Capabilities")) {
        return FALSE;
    }
    
    // Register as StartMenuInternet client
    snprintf(keyPath, sizeof(keyPath), "Software\\Clients\\StartMenuInternet\\%s", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, NULL, APP_DESC)) {
        return FALSE;
    }
    
    // Capabilities
    snprintf(keyPath, sizeof(keyPath), "Software\\Clients\\StartMenuInternet\\%s\\Capabilities", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, "ApplicationName", APP_DESC)) {
        return FALSE;
    }
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, "ApplicationDescription", "Select which browser to use for opening URLs")) {
        return FALSE;
    }
    
    // URL Associations
    snprintf(keyPath, sizeof(keyPath), "Software\\Clients\\StartMenuInternet\\%s\\Capabilities\\URLAssociations", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, "http", APP_NAME)) {
        return FALSE;
    }
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, "https", APP_NAME)) {
        return FALSE;
    }
    
    // Shell open command
    snprintf(keyPath, sizeof(keyPath), "Software\\Clients\\StartMenuInternet\\%s\\shell\\open\\command", APP_NAME);
    if (!SetRegistryValue(HKEY_CURRENT_USER, keyPath, NULL, command)) {
        return FALSE;
    }
    
    // Notify shell of changes
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    
    return TRUE;
}

BOOL UnregisterAsBrowser(void) {
    char keyPath[256];
    
    // Delete main class registration
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\%s", APP_NAME);
    DeleteRegistryKey(HKEY_CURRENT_USER, keyPath);
    
    // Delete http handler
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\http\\shell\\%s", APP_NAME);
    DeleteRegistryKey(HKEY_CURRENT_USER, keyPath);
    
    // Delete https handler
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\https\\shell\\%s", APP_NAME);
    DeleteRegistryKey(HKEY_CURRENT_USER, keyPath);
    
    // Delete from RegisteredApplications
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\RegisteredApplications", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, APP_NAME);
        RegCloseKey(hKey);
    }
    
    // Delete StartMenuInternet client
    snprintf(keyPath, sizeof(keyPath), "Software\\Clients\\StartMenuInternet\\%s", APP_NAME);
    DeleteRegistryKey(HKEY_CURRENT_USER, keyPath);
    
    // Notify shell of changes
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    
    return TRUE;
}

BOOL IsRegisteredAsBrowser(void) {
    char keyPath[256];
    HKEY hKey;
    
    snprintf(keyPath, sizeof(keyPath), "Software\\Classes\\%s", APP_NAME);
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_READ, &hKey);
    
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return TRUE;
    }
    
    return FALSE;
}

BOOL IsDefaultBrowser(void) {
    char currentDefault[256];
    DWORD bufferSize = sizeof(currentDefault);
    HKEY hKey;
    
    // Check http default
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, 
                                "Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice",
                                0, KEY_READ, &hKey);
    
    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExA(hKey, "ProgId", NULL, NULL, 
                                 (LPBYTE)currentDefault, &bufferSize);
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS && strcmp(currentDefault, APP_NAME) == 0) {
            // Check https as well
            bufferSize = sizeof(currentDefault);
            result = RegOpenKeyExA(HKEY_CURRENT_USER,
                                  "Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice",
                                  0, KEY_READ, &hKey);
            
            if (result == ERROR_SUCCESS) {
                result = RegQueryValueExA(hKey, "ProgId", NULL, NULL,
                                         (LPBYTE)currentDefault, &bufferSize);
                RegCloseKey(hKey);
                
                if (result == ERROR_SUCCESS && strcmp(currentDefault, APP_NAME) == 0) {
                    return TRUE;
                }
            }
        }
    }
    
    return FALSE;
}

BOOL SetAsDefaultBrowser(void) {
    // Try to set default using IApplicationAssociationRegistration (Windows Vista+)
    IApplicationAssociationRegistration* pAAR = NULL;
    
    HRESULT hr = CoInitialize(NULL);
    BOOL comInitialized = SUCCEEDED(hr);
    
    hr = CoCreateInstance(&CLSID_ApplicationAssociationRegistration, NULL,
                         CLSCTX_INPROC, &IID_IApplicationAssociationRegistration,
                         (void**)&pAAR);
    
    if (SUCCEEDED(hr) && pAAR) {
        // Set as default for http
        WCHAR appName[256];
        MultiByteToWideChar(CP_ACP, 0, APP_NAME, -1, appName, 256);
        
        hr = pAAR->lpVtbl->SetAppAsDefault(pAAR, appName, L"http", AT_URLPROTOCOL);
        
        if (SUCCEEDED(hr)) {
            // Set as default for https
            hr = pAAR->lpVtbl->SetAppAsDefault(pAAR, appName, L"https", AT_URLPROTOCOL);
        }
        
        pAAR->lpVtbl->Release(pAAR);
        
        if (comInitialized) {
            CoUninitialize();
        }
        
        if (SUCCEEDED(hr)) {
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
            return TRUE;
        }
    }
    
    if (comInitialized) {
        CoUninitialize();
    }
    
    // If API method failed, open Windows Settings for user to select manually
    // This is required on Windows 10+ due to security restrictions
    ShellExecuteA(NULL, "open", "ms-settings:defaultapps", NULL, NULL, SW_SHOW);
    
    return FALSE; // Indicate user needs to complete manually
}
