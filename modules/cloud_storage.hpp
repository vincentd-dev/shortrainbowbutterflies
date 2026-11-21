#ifndef VAULT_H // Prevents multiple inclusion of vault.h
#define VAULT_H // Marks VAULT_H as defined

#include <string> // Enables string parameter

// CLOUD STORAGE: Cloud vault with backups, version history, auto-backup, sharing, and sync status
void runCloudStorageModule(const std::string& username);

#endif // End of VAULT_H