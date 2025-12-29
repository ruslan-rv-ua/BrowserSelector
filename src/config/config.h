#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PATH_LENGTH 260
#define MAX_NAME_LENGTH 100
#define MAX_COMMAND_LENGTH 512
#define MAX_COMMANDS 50

// LoadConfig return codes
#define CONFIG_OK               0
#define CONFIG_FILE_NOT_FOUND   1
#define CONFIG_PARSE_ERROR      2
#define CONFIG_READ_ERROR       3
#define CONFIG_MEMORY_ERROR     4

typedef struct {
    char name[MAX_NAME_LENGTH];
    char command[MAX_COMMAND_LENGTH];
} Command;

typedef struct {
    int defaultCommandIndex;
    int waitTime;
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
