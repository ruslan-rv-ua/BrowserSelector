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

// Check if application is the default browser for http/https
BOOL IsDefaultBrowser(void);

// Set application as default browser (requires prior registration)
// Returns TRUE on success
BOOL SetAsDefaultBrowser(void);

#endif // REGISTRY_H
