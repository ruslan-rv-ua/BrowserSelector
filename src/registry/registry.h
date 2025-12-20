#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>

// Register application as a browser handler
// Returns TRUE on success
BOOL RegisterAsBrowser(const char* exePath);

// Unregister application as a browser handler
// Returns TRUE on success
BOOL UnregisterAsBrowser(void);

// Check if application is registered as browser
BOOL IsRegisteredAsBrowser(void);

#endif // REGISTRY_H
