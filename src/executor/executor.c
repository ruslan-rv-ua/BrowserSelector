#include "executor.h"
#include "../i18n/i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static char* ReplaceUrlPlaceholder(const char* command, const char* url) {
    if (!url) url = "";
    
    // Find {url} placeholder
    const char* pos = strstr(command, "{url}");
    if (!pos) {
        return _strdup(command);
    }
    
    // Calculate result size
    size_t urlLen = strlen(url);
    size_t cmdLen = strlen(command);
    size_t resultSize = cmdLen - 5 + urlLen + 1;
    
    char* result = (char*)malloc(resultSize);
    if (!result) return NULL;
    
    // Copy part before {url}
    size_t prefixLen = pos - command;
    strncpy(result, command, prefixLen);
    result[prefixLen] = '\0';
    
    // Add URL
    strcat(result, url);
    
    // Add part after {url}
    strcat(result, pos + 5);
    
    return result;
}

int ExecuteCommand(const Command* command, const char* url) {
    // Replace {url} placeholder in the command string
    char* cmdLine = ReplaceUrlPlaceholder(command->command, url);
    if (!cmdLine) {
        MessageBoxW(NULL, I18n_GetStringW(IDS_MEMORY_ERROR), 
                   I18n_GetStringW(IDS_ERROR), MB_OK | MB_ICONERROR);
        return 0;
    }
    
    // Launch process using cmd.exe /c to support shell commands and pipes
    char fullCmd[2048];
    snprintf(fullCmd, sizeof(fullCmd), "cmd.exe /c %s", cmdLine);
    
    // Launch process
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    
    BOOL success = CreateProcessA(
        NULL,           // lpApplicationName
        fullCmd,        // lpCommandLine
        NULL,           // lpProcessAttributes
        NULL,           // lpThreadAttributes
        FALSE,          // bInheritHandles
        CREATE_NO_WINDOW,  // dwCreationFlags - hide console window
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
    
    free(cmdLine);
    
    return success ? 1 : 0;
}
