#include "executor.h"
#include "../i18n/i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static char* ReplaceUrlPlaceholder(const char* arguments, const char* url) {
    if (!url) url = "";
    
    // Find {url} placeholder
    const char* pos = strstr(arguments, "{url}");
    if (!pos) {
        return _strdup(arguments);
    }
    
    // Calculate result size
    size_t urlLen = strlen(url);
    size_t argsLen = strlen(arguments);
    size_t resultSize = argsLen - 5 + urlLen + 1;
    
    char* result = (char*)malloc(resultSize);
    if (!result) return NULL;
    
    // Copy part before {url}
    size_t prefixLen = pos - arguments;
    strncpy(result, arguments, prefixLen);
    result[prefixLen] = '\0';
    
    // Add URL
    strcat(result, url);
    
    // Add part after {url}
    strcat(result, pos + 5);
    
    return result;
}

static char* ResolveCommand(const char* command) {
    char resolved[MAX_PATH];
    
    // Check if it's an absolute path (contains : or starts with \)
    if (strchr(command, ':') || command[0] == '\\') {
        DWORD attrs = GetFileAttributesA(command);
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return _strdup(command);
        }
        return NULL;
    }
    
    // Check if it's a relative path (contains \ or /)
    if (strchr(command, '\\') || strchr(command, '/')) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            // Use safe path combination
            size_t exeLen = strlen(exePath);
            size_t cmdLen = strlen(command);
            if (exeLen + cmdLen + 2 < MAX_PATH) {
                int written = snprintf(resolved, MAX_PATH, "%s\\%s", exePath, command);
                if (written < 0 || written >= MAX_PATH) {
                    return NULL;  // Truncation occurred
                }
                
                DWORD attrs = GetFileAttributesA(resolved);
                if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    return _strdup(resolved);
                }
            }
        }
        return NULL;
    }
    
    // Search in PATH
    if (SearchPathA(NULL, command, ".exe", MAX_PATH, resolved, NULL)) {
        return _strdup(resolved);
    }
    
    // Try with .exe extension
    if (strlen(command) + 4 < MAX_PATH) {
        char commandWithExt[MAX_PATH];
        int written = snprintf(commandWithExt, MAX_PATH, "%s.exe", command);
        if (written > 0 && written < MAX_PATH) {
            if (SearchPathA(NULL, commandWithExt, NULL, MAX_PATH, resolved, NULL)) {
                return _strdup(resolved);
            }
        }
    }
    
    return NULL;
}

int ExecuteCommand(const Command* command, const char* url) {
    // Prepare arguments with URL substitution
    char* args = ReplaceUrlPlaceholder(command->arguments, url);
    if (!args) {
        MessageBoxW(NULL, I18n_GetStringW(IDS_MEMORY_ERROR), 
                   I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
        return 0;
    }
    
    // Resolve command path
    char* cmdPath = ResolveCommand(command->command);
    if (!cmdPath) {
        wchar_t error[1024];
        wchar_t wideCmd[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, command->command, -1, wideCmd, MAX_PATH);
        
        swprintf(error, 1024, L"%s\n\n%s", 
                I18n_GetStringW(IDS_CMD_NOT_FOUND), 
                I18n_GetStringW(IDS_CMD_NOT_FOUND_HINT));
        
        // Replace %s with command name
        wchar_t finalError[1024];
        swprintf(finalError, 1024, error, wideCmd);
        
        MessageBoxW(NULL, finalError, I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
        free(args);
        return 0;
    }
    
    // Prepare command line
    char cmdLine[2048];
    snprintf(cmdLine, sizeof(cmdLine), "\"%s\" %s", cmdPath, args);
    
    // Launch process
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
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
        wchar_t error[1024];
        wchar_t wideCmd[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, command->command, -1, wideCmd, MAX_PATH);
        
        swprintf(error, 1024, I18n_GetStringW(IDS_LAUNCH_FAILED), wideCmd, GetLastError());
        MessageBoxW(NULL, error, I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
    }
    
    free(cmdPath);
    free(args);
    
    return success ? 1 : 0;
}
