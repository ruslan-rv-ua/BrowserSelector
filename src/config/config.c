#include "config.h"
#include "../../include/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void GetConfigPath(const char* exePath, char* configPath) {
    strcpy(configPath, exePath);
    char* lastSlash = strrchr(configPath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
        strcat(configPath, "config.json");
    }
}

int CreateDefaultConfig(const char* configPath) {
    // Create JSON object
    cJSON* json = cJSON_CreateObject();
    
    // Add settings
    cJSON* settings = cJSON_CreateObject();
    cJSON_AddNumberToObject(settings, "defaultCommandIndex", 3);  // Helium as default
    cJSON_AddNumberToObject(settings, "waitTime", 10);
    cJSON_AddItemToObject(json, "settings", settings);
    
    // Add default commands
    cJSON* commands = cJSON_CreateArray();
    
    // Copy to Clipboard (default)
    cJSON* clipboard = cJSON_CreateObject();
    cJSON_AddStringToObject(clipboard, "name", "Copy to Clipboard");
    cJSON_AddStringToObject(clipboard, "command", "echo {url} | clip");
    cJSON_AddItemToArray(commands, clipboard);

    // Chrome Incognito
    cJSON* chrome = cJSON_CreateObject();
    cJSON_AddStringToObject(chrome, "name", "Chrome Incognito");
    cJSON_AddStringToObject(chrome, "command", "chrome.exe --incognito {url}");
    cJSON_AddItemToArray(commands, chrome);

    // Firefox with profile
    cJSON* firefox = cJSON_CreateObject();
    cJSON_AddStringToObject(firefox, "name", "Firefox Profile");
    cJSON_AddStringToObject(firefox, "command", "firefox.exe -P default {url}");
    cJSON_AddItemToArray(commands, firefox);
    
    // Helium
    cJSON* helium = cJSON_CreateObject();
    cJSON_AddStringToObject(helium, "name", "Helium");
    cJSON_AddStringToObject(helium, "command", "helium.exe {url}");
    cJSON_AddItemToArray(commands, helium);
    
    cJSON_AddItemToObject(json, "commands", commands);
    
    // Write to file
    char* jsonString = cJSON_Print(json);
    FILE* file = fopen(configPath, "w");
    int success = 0;
    if (file) {
        fputs(jsonString, file);
        fclose(file);
        success = 1;
    }
    
    cJSON_free(jsonString);
    cJSON_Delete(json);
    
    return success;
}

int LoadConfig(const char* exePath, Configuration* config) {
    // Get config.json path
    char configPath[MAX_PATH];
    GetConfigPath(exePath, configPath);
    
    // Try to open file
    FILE* file = fopen(configPath, "r");
    if (!file) {
        // File doesn't exist - silently create default configuration
        if (CreateDefaultConfig(configPath)) {
            file = fopen(configPath, "r");
            if (!file) return CONFIG_READ_ERROR;
        } else {
            return CONFIG_READ_ERROR;
        }
    }
    
    // Read file
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* jsonString = (char*)malloc(fileSize + 1);
    if (!jsonString) {
        fclose(file);
        return CONFIG_MEMORY_ERROR;
    }
    
    size_t bytesRead = fread(jsonString, 1, fileSize, file);
    jsonString[bytesRead] = '\0';
    fclose(file);
    
    // Parse JSON
    cJSON* json = cJSON_Parse(jsonString);
    free(jsonString);
    
    if (!json) {
        return CONFIG_PARSE_ERROR;
    }
    
    // Initialize defaults
    config->settings.defaultCommandIndex = 0;
    config->settings.waitTime = 10;
    config->commandCount = 0;
    
    // Read settings
    cJSON* settings = cJSON_GetObjectItem(json, "settings");
    if (settings) {
        cJSON* defaultIndex = cJSON_GetObjectItem(settings, "defaultCommandIndex");
        cJSON* waitTime = cJSON_GetObjectItem(settings, "waitTime");
        
        if (defaultIndex && cJSON_IsNumber(defaultIndex)) {
            config->settings.defaultCommandIndex = defaultIndex->valueint;
        }
        
        if (waitTime && cJSON_IsNumber(waitTime)) {
            config->settings.waitTime = waitTime->valueint;
        }
    }
    
    // Read commands
    cJSON* commands = cJSON_GetObjectItem(json, "commands");
    if (commands && cJSON_IsArray(commands)) {
        cJSON* command = NULL;
        cJSON_ArrayForEach(command, commands) {
            if (config->commandCount >= MAX_COMMANDS) break;
            
            cJSON* name = cJSON_GetObjectItem(command, "name");
            cJSON* cmd = cJSON_GetObjectItem(command, "command");
            
            if (name && cJSON_IsString(name) && 
                cmd && cJSON_IsString(cmd)) {
                strncpy(config->commands[config->commandCount].name, 
                       name->valuestring, MAX_NAME_LENGTH - 1);
                config->commands[config->commandCount].name[MAX_NAME_LENGTH - 1] = '\0';
                
                strncpy(config->commands[config->commandCount].command, 
                       cmd->valuestring, MAX_COMMAND_LENGTH - 1);
                config->commands[config->commandCount].command[MAX_COMMAND_LENGTH - 1] = '\0';
                
                config->commandCount++;
            }
        }
    }
    
    cJSON_Delete(json);
    
    // Validate defaultCommandIndex
    if (config->settings.defaultCommandIndex >= config->commandCount) {
        config->settings.defaultCommandIndex = 0;
    }
    
    return CONFIG_OK;
}

int SaveConfig(const char* exePath, const Configuration* config) {
    // Get config.json path
    char configPath[MAX_PATH];
    GetConfigPath(exePath, configPath);
    
    // Create JSON object
    cJSON* json = cJSON_CreateObject();
    
    // Add settings
    cJSON* settings = cJSON_CreateObject();
    cJSON_AddNumberToObject(settings, "defaultCommandIndex",
                           config->settings.defaultCommandIndex);
    cJSON_AddNumberToObject(settings, "waitTime",
                           config->settings.waitTime);
    cJSON_AddItemToObject(json, "settings", settings);
    
    // Add commands
    cJSON* commands = cJSON_CreateArray();
    for (int i = 0; i < config->commandCount; i++) {
        cJSON* command = cJSON_CreateObject();
        cJSON_AddStringToObject(command, "name", config->commands[i].name);
        cJSON_AddStringToObject(command, "command", config->commands[i].command);
        cJSON_AddItemToArray(commands, command);
    }
    cJSON_AddItemToObject(json, "commands", commands);
    
    // Write to file
    char* jsonString = cJSON_Print(json);
    FILE* file = fopen(configPath, "w");
    int success = 0;
    if (file) {
        fputs(jsonString, file);
        fclose(file);
        success = 1;
    }
    
    cJSON_free(jsonString);
    cJSON_Delete(json);
    
    return success;
}
