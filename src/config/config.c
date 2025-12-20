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
    cJSON_AddNumberToObject(settings, "defaultCommandIndex", 0);
    cJSON_AddItemToObject(json, "settings", settings);
    
    // Add default commands
    cJSON* commands = cJSON_CreateArray();
    
    // LibreWolf (default, first)
    cJSON* librewolf = cJSON_CreateObject();
    cJSON_AddStringToObject(librewolf, "name", "LibreWolf");
    cJSON_AddStringToObject(librewolf, "command", "librewolf.exe");
    cJSON_AddStringToObject(librewolf, "arguments", "{url}");
    cJSON_AddItemToArray(commands, librewolf);

    // Chrome (incognito)
    cJSON* chrome = cJSON_CreateObject();
    cJSON_AddStringToObject(chrome, "name", "Chrome");
    cJSON_AddStringToObject(chrome, "command", "chrome.exe");
    // Launch Chrome in incognito mode by default
    cJSON_AddStringToObject(chrome, "arguments", "--incognito {url}");
    cJSON_AddItemToArray(commands, chrome);
    
    // Helium
    cJSON* helium = cJSON_CreateObject();
    cJSON_AddStringToObject(helium, "name", "Helium");
    cJSON_AddStringToObject(helium, "command", "helium.exe");
    cJSON_AddStringToObject(helium, "arguments", "{url}");
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
        // Create default configuration
        if (CreateDefaultConfig(configPath)) {
            file = fopen(configPath, "r");
            if (!file) return 0;
        } else {
            return 0;
        }
    }
    
    // Read file
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* jsonString = (char*)malloc(fileSize + 1);
    if (!jsonString) {
        fclose(file);
        return 0;
    }
    
    size_t bytesRead = fread(jsonString, 1, fileSize, file);
    jsonString[bytesRead] = '\0';
    fclose(file);
    
    // Parse JSON
    cJSON* json = cJSON_Parse(jsonString);
    free(jsonString);
    
    if (!json) {
        return 0;
    }
    
    // Initialize defaults
    config->settings.defaultCommandIndex = 0;
    config->commandCount = 0;
    
    // Read settings
    cJSON* settings = cJSON_GetObjectItem(json, "settings");
    if (settings) {
        cJSON* defaultIndex = cJSON_GetObjectItem(settings, "defaultCommandIndex");
        
        if (defaultIndex && cJSON_IsNumber(defaultIndex)) {
            config->settings.defaultCommandIndex = defaultIndex->valueint;
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
            cJSON* args = cJSON_GetObjectItem(command, "arguments");
            
            if (name && cJSON_IsString(name) && 
                cmd && cJSON_IsString(cmd) && 
                args && cJSON_IsString(args)) {
                strncpy(config->commands[config->commandCount].name, 
                       name->valuestring, MAX_NAME_LENGTH - 1);
                config->commands[config->commandCount].name[MAX_NAME_LENGTH - 1] = '\0';
                
                strncpy(config->commands[config->commandCount].command, 
                       cmd->valuestring, MAX_PATH_LENGTH - 1);
                config->commands[config->commandCount].command[MAX_PATH_LENGTH - 1] = '\0';
                
                strncpy(config->commands[config->commandCount].arguments, 
                       args->valuestring, MAX_PATH_LENGTH - 1);
                config->commands[config->commandCount].arguments[MAX_PATH_LENGTH - 1] = '\0';
                
                config->commandCount++;
            }
        }
    }
    
    cJSON_Delete(json);
    
    // Validate defaultCommandIndex
    if (config->settings.defaultCommandIndex >= config->commandCount) {
        config->settings.defaultCommandIndex = 0;
    }
    
    return 1;
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
    cJSON_AddItemToObject(json, "settings", settings);
    
    // Add commands
    cJSON* commands = cJSON_CreateArray();
    for (int i = 0; i < config->commandCount; i++) {
        cJSON* command = cJSON_CreateObject();
        cJSON_AddStringToObject(command, "name", config->commands[i].name);
        cJSON_AddStringToObject(command, "command", config->commands[i].command);
        cJSON_AddStringToObject(command, "arguments", config->commands[i].arguments);
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
