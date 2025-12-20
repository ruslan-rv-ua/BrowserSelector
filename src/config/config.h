#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PATH_LENGTH 260
#define MAX_NAME_LENGTH 100
#define MAX_COMMANDS 50

typedef struct {
    char name[MAX_NAME_LENGTH];
    char command[MAX_PATH_LENGTH];
    char arguments[MAX_PATH_LENGTH];
} Command;

typedef struct {
    int defaultCommandIndex;
    int windowWidth;
    int windowHeight;
} Settings;

typedef struct {
    Settings settings;
    Command commands[MAX_COMMANDS];
    int commandCount;
} Configuration;

// Functions for working with configuration
int LoadConfig(const char* exePath, Configuration* config);
int SaveConfig(const char* exePath, const Configuration* config);
int CreateDefaultConfig(const char* configPath);
void GetConfigPath(const char* exePath, char* configPath);

#endif // CONFIG_H
