#include "cloud_storage.hpp"   // Declares runCloudStorageModule()
#include <ctime>     // Provides time, tm, strftime
#include <filesystem>// Provides filesystem directory/file operations
#include <fstream>   // Provides ifstream and ofstream
#include <iomanip>   // Provides setw, left, right
#include <iostream>  // Provides cin and cout
#include <limits>    // Provides numeric_limits
#include <string>    // Provides string

using namespace std; // Brings std names into vault.cpp
namespace fs = filesystem; // Short alias for std::filesystem

namespace { // Private cloud vault helpers

// CLOUD STORAGE: Generates timestamp string YYYYMMDD_HHMMSS for versioned snapshots
string currentTimestamp() {
    time_t now = time(nullptr);
    tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &now);
#else
    localtime_r(&now, &tmBuf);
#endif
    char buf[32];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tmBuf);
    return string(buf);
}

// CLOUD STORAGE: Creates a versioned snapshot in data/backups/ and logs version history
bool createVersionBackup(const string& username, const string& sourcePath, const string& modeLabel) {
    if (!fs::exists(sourcePath)) {
        cout << "[CLOUD STORAGE] Source file not found: " << sourcePath << "\n";
        return false;
    }

    fs::create_directories("data/backups");
    fs::path src(sourcePath);
    string stamp = currentTimestamp();
    string backupName = src.stem().string() + "_" + username + "_" + stamp + src.extension().string();
    fs::path dest = fs::path("data/backups") / backupName;

    fs::copy_file(src, dest, fs::copy_options::overwrite_existing);

    ofstream log("data/backups/version_history.log", ios::app);
    if (log.is_open()) {
        log << stamp << " | mode=" << modeLabel
            << " | user=" << username
            << " | source=" << sourcePath
            << " | snapshot=" << backupName << "\n";
    }

    cout << "[CLOUD STORAGE] " << modeLabel << " saved: " << dest.string() << "\n";
    return true;
}

// CLOUD STORAGE: Lists backup snapshots and shows version history log
void listBackupsAndHistory() {
    cout << "\n--- CLOUD STORAGE: SNAPSHOTS & VERSION HISTORY ---\n";
    if (!fs::exists("data/backups")) {
        cout << "No backup directory found.\n";
        return;
    }

    cout << left << setw(42) << "Snapshot File" << right << setw(12) << "Size (B)" << "\n";
    cout << string(54, '-') << "\n";

    bool any = false;
    for (const auto& entry : fs::directory_iterator("data/backups")) {
        if (entry.is_regular_file()) {
            cout << left << setw(42) << entry.path().filename().string()
                 << right << setw(12) << entry.file_size() << "\n";
            any = true;
        }
    }
    if (!any) cout << "No backup snapshots yet.\n";

    ifstream log("data/backups/version_history.log");
    if (log.is_open()) {
        cout << "\n--- Version & Sync Log ---\n";
        string line;
        while (getline(log, line)) {
            if (!line.empty()) cout << " * " << line << "\n";
        }
    }
}

// CLOUD STORAGE: Simulates file sharing permissions and cross-device sync manifest
void shareAndSyncMenu(const string& username) {
    string targetFile, targetUser, perm;
    cout << "File name/path to share & sync: ";
    getline(cin, targetFile);
    cout << "Share with user/role (e.g., tutor1 or Team): ";
    getline(cin, targetUser);
    cout << "Permission (Read / Edit): ";
    getline(cin, perm);

    ofstream out("data/cloud_shares.txt", ios::app);
    if (out.is_open()) {
        out << currentTimestamp() << "|owner=" << username
            << "|file=" << targetFile << "|shared_with=" << targetUser
            << "|perm=" << perm << "|sync=SYNCED_LOCAL\n";
        cout << "[CLOUD STORAGE] Shared '" << targetFile << "' with '" << targetUser
             << "' (" << perm << ") and marked synced.\n";
    }
}

} // namespace

// CLOUD STORAGE: Interactive menu for backups, auto-backup, restore, sharing, and sync
void runCloudStorageModule(const string& username) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- CLOUD STORAGE (SIMULATED VAULT) ---\n";
        cout << "[1] Backup Spreadsheet (data/sheet_data.csv)\n";
        cout << "[2] Auto-Backup Notes & Schedule Files\n";
        cout << "[3] View Backups, Version History & Sync Log\n";
        cout << "[4] Restore Snapshot to Target File\n";
        cout << "[5] File Sharing Permissions & Cross-Device Sync\n";
        cout << "[0] Back to Dashboard\n";
        cout << "Choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            createVersionBackup(username, "data/sheet_data.csv", "MANUAL_BACKUP");
        } else if (choice == 2) {
            createVersionBackup(username, "data/notes.txt", "AUTO_BACKUP");
            createVersionBackup(username, "data/schedule.txt", "AUTO_BACKUP");
        } else if (choice == 3) {
            listBackupsAndHistory();
        } else if (choice == 4) {
            string snapshotName, restorePath;
            cout << "Snapshot filename in data/backups/: ";
            getline(cin, snapshotName);
            cout << "Restore to path (e.g., data/sheet_data.csv): ";
            getline(cin, restorePath);

            fs::path src = fs::path("data/backups") / snapshotName;
            if (!fs::exists(src)) {
                cout << "[CLOUD STORAGE] Snapshot does not exist.\n";
            } else {
                if (fs::path(restorePath).has_parent_path()) {
                    fs::create_directories(fs::path(restorePath).parent_path());
                }
                fs::copy_file(src, restorePath, fs::copy_options::overwrite_existing);
                cout << "[CLOUD STORAGE] Restored '" << snapshotName << "' -> '" << restorePath << "'\n";
            }
        } else if (choice == 5) {
            shareAndSyncMenu(username);
        }
    }
}
