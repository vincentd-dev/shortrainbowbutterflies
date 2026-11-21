#ifndef VAULT_H // Prevents multiple inclusion of vault.h
#define VAULT_H // Marks VAULT_H as defined

#include <string> // Enables string parameter

using namespace std; // Enables standard library usage in header

// CLOUD STORAGE: Simulated cloud vault with backups, version history, auto-backup, sharing, and sync status
void runCloudStorageModule(const string& username);

#endif // End of VAULT_H
