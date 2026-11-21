#include "user_accounts_security.hpp"    // Declares Session and auth functions
#include <filesystem>// Provides filesystem directory/file checks
#include <fstream>   // Provides ifstream and ofstream for file persistence
#include <iomanip>   // Provides fixed, setprecision, setw, and left formatting
#include <iostream>  // Provides cin and cout for console I/O
#include <limits>    // Provides numeric_limits for input clearing
#include <random>    // Provides random_device, mt19937, and uniform_int_distribution
#include <sstream>   // Provides stringstream for pipe-delimited parsing
#include <vector>    // Provides vector for updating records

using namespace std; // Brings std names into auth.cpp
namespace fs = filesystem; // Short alias for std::filesystem

namespace { // Private helpers internal to auth.cpp

// AUTH & SECURITY: Simple reversible cipher to encode/decode passwords in local storage
string transformCipher(const string& input, bool encrypt) {
    string out = input; // Copy input string so original stays intact
    int direction = encrypt ? 1 : -1; // +1 to encode, -1 to decode
    for (size_t i = 0; i < out.size(); ++i) {
        int shift = (3 + static_cast<int>(i % 5)) * direction; // Position-based shift
        unsigned char u = static_cast<unsigned char>(out[i]);
        out[i] = static_cast<char>(static_cast<unsigned int>(u + shift) & 0xFFu);
    }
    return out;
}

// AUTH & SECURITY: Checks whether a username already exists in data/users.txt
bool userExists(const string& targetUser) {
    ifstream in("data/users.txt"); // Open users database
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string user;
        if (getline(ss, user, '|') && user == targetUser) {
            return true; // Duplicate username found
        }
    }
    return false;
}

// AUTH & SECURITY: Prompts user to choose a role-based permission tier
string chooseRole() {
    int choice = 0; // Menu choice for role
    while (true) {
        cout << "\nSelect Role:\n";
        cout << "[1] Student\n";
        cout << "[2] Tutor\n";
        cout << "[3] Admin\n";
        cout << "[4] Engineer\n";
        cout << "Choice: ";
        if (cin >> choice) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            switch (choice) {
                case 1: return "Student";
                case 2: return "Tutor";
                case 3: return "Admin";
                case 4: return "Engineer";
                default: cout << "Invalid choice. Try again.\n";
            }
        } else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid number.\n";
        }
    }
}

// AUTH & SECURITY: Generates a 6-digit MFA code and saves it to data/mfa_inbox.txt
int generateAndSaveMfa(const string& username) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(100000, 999999);
    int code = dist(gen); // Fresh 6-digit code (100000..999999)

    fs::create_directories("data");
    ofstream out("data/mfa_inbox.txt", ios::app); // Append MFA message
    if (out.is_open()) {
        out << "User: " << username << " | MFA Code: " << code << "\n";
    }
    return code;
}

} // namespace

// AUTH & SECURITY: Creates data folders and seeds default accounts if missing
void initAuthFiles() {
    fs::create_directories("data");         // Create main data folder
    fs::create_directories("data/backups"); // Create backup folder for cloud vault

    if (!fs::exists("data/users.txt")) { // Seed default accounts (password: pass123)
        ofstream users("data/users.txt");
        users << "student1|" << transformCipher("pass123", true) << "|Student|500.00|BSIT/Engineering Student|0\n";
        users << "tutor1|" << transformCipher("pass123", true) << "|Tutor|1200.00|Math & Physics Tutor|0\n";
        users << "admin1|" << transformCipher("pass123", true) << "|Admin|0.00|System Administrator|1\n";
    }

    if (!fs::exists("data/mfa_inbox.txt")) { // Create MFA inbox file
        ofstream mfa("data/mfa_inbox.txt");
    }
}

// AUTH & SECURITY: Creates a new user account with encoded password, role, and profile
bool registerUser() {
    string username, password, bio;
    cout << "\n--- Sign Up ---\n";
    cout << "Enter username: ";
    getline(cin >> ws, username);

    if (username.empty() || username.find('|') != string::npos) {
        cout << "Invalid username (cannot be empty or contain '|').\n";
        return false;
    }
    if (userExists(username)) {
        cout << "That username is already taken.\n";
        return false;
    }

    cout << "Create password: ";
    getline(cin, password);
    if (password.empty() || password.find('|') != string::npos) {
        cout << "Invalid password.\n";
        return false;
    }

    string role = chooseRole(); // Assign user role
    cout << "Short profile bio/title: ";
    getline(cin, bio);
    if (bio.empty()) bio = role + " member";
    for (char& ch : bio) {
        if (ch == '|') ch = '/'; // Avoid breaking pipe-delimited record format
    }

    double startingBalance = (role == "Student") ? 500.00 : 0.00; // Starting wallet balance

    fs::create_directories("data");
    ofstream out("data/users.txt", ios::app);
    if (!out.is_open()) {
        cout << "Error saving user account.\n";
        return false;
    }

    out << username << "|" << transformCipher(password, true) << "|"
        << role << "|" << fixed << setprecision(2) << startingBalance
        << "|" << bio << "|0\n";

    cout << "Account created for '" << username << "' as [" << role << "]!\n";
    return true;
}

// AUTH & SECURITY: Logs in user with encoded password check + 6-digit MFA verification
bool loginUser(Session& activeSession) {
    string username, password;
    cout << "\n--- Login ---\n";
    cout << "Username: ";
    getline(cin >> ws, username);
    cout << "Password: ";
    getline(cin, password);

    ifstream in("data/users.txt");
    if (!in.is_open()) {
        cout << "Could not open users database.\n";
        return false;
    }

    string expectedEncoded = transformCipher(password, true); // Encode entered password for comparison
    string line;
    bool found = false;
    string role, balanceStr, bio, privStr;

    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string u, p;
        if (getline(ss, u, '|') &&
            getline(ss, p, '|') &&
            getline(ss, role, '|') &&
            getline(ss, balanceStr, '|')) {
            getline(ss, bio, '|');
            getline(ss, privStr, '|');
            if (u == username && p == expectedEncoded) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        cout << "Invalid username or password.\n";
        return false;
    }

    int mfaCode = generateAndSaveMfa(username); // Generate and log MFA code
    cout << "\n[MFA] Verification code written to 'data/mfa_inbox.txt'.\n";
    cout << "[MFA Code]: " << mfaCode << "\n";

    int enteredCode = 0;
    cout << "Enter 6-digit MFA code: ";
    if (!(cin >> enteredCode)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid MFA input. Login cancelled.\n";
        return false;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (enteredCode != mfaCode) {
        cout << "MFA verification failed.\n";
        return false;
    }

    activeSession.loggedIn = true;
    activeSession.username = username;
    activeSession.role = role;
    activeSession.profileBio = bio.empty() ? (role + " user") : bio;
    activeSession.privateProfile = (privStr == "1");
    try {
        activeSession.walletBalance = stod(balanceStr);
    } catch (...) {
        activeSession.walletBalance = 0.0;
    }

    cout << "\nWelcome, " << activeSession.username << " (" << activeSession.role << ")!\n";
    return true;
}

// AUTH & PROFILES: Displays and updates profile bio, privacy setting, and encryption status
void manageProfileAndPrivacy(Session& activeSession) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- USER PROFILE, PRIVACY & ENCRYPTION ---\n";
        cout << left << setw(22) << "Username:" << activeSession.username << "\n";
        cout << left << setw(22) << "Role:" << activeSession.role << "\n";
        cout << left << setw(22) << "Profile Bio:"
             << (activeSession.privateProfile ? "[Hidden - Private Mode]" : activeSession.profileBio) << "\n";
        cout << left << setw(22) << "Privacy Mode:"
             << (activeSession.privateProfile ? "Private (ON)" : "Standard (OFF)") << "\n";
        cout << left << setw(22) << "Credential Cipher:" << "Active (Local Shift Encoding)\n";
        cout << left << setw(22) << "Wallet Balance:" << "PHP " << fixed << setprecision(2)
             << activeSession.walletBalance << "\n";
        cout << "\n[1] Update Profile Bio\n";
        cout << "[2] Toggle Privacy Mode\n";
        cout << "[3] Top Up Wallet (+PHP 250.00)\n";
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
            string newBio;
            cout << "New profile bio: ";
            getline(cin, newBio);
            if (!newBio.empty()) {
                for (char& ch : newBio) if (ch == '|') ch = '/';
                activeSession.profileBio = newBio;
                updateWalletBalance(activeSession.username, activeSession.walletBalance);
                cout << "Profile bio updated.\n";
            }
        } else if (choice == 2) {
            activeSession.privateProfile = !activeSession.privateProfile;
            updateWalletBalance(activeSession.username, activeSession.walletBalance);
            cout << "Privacy mode is now " << (activeSession.privateProfile ? "ON" : "OFF") << ".\n";
        } else if (choice == 3) {
            activeSession.walletBalance += 250.00;
            updateWalletBalance(activeSession.username, activeSession.walletBalance);
            cout << "Added PHP 250.00 to wallet.\n";
        }
    }
}

// AUTH & PROFILES: Rewrites data/users.txt with updated wallet balance and session profile flags
bool updateWalletBalance(const string& username, double newBalance) {
    ifstream in("data/users.txt");
    if (!in.is_open()) return false;

    vector<string> lines;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string u, p, role, bal, bio, priv;
        getline(ss, u, '|');
        getline(ss, p, '|');
        getline(ss, role, '|');
        getline(ss, bal, '|');
        getline(ss, bio, '|');
        getline(ss, priv, '|');

        if (u == username) {
            ostringstream updated;
            updated << u << "|" << p << "|" << role << "|"
                    << fixed << setprecision(2) << newBalance << "|"
                    << (bio.empty() ? "Member" : bio) << "|"
                    << (priv.empty() ? "0" : priv);
            lines.push_back(updated.str());
        } else {
            lines.push_back(line);
        }
    }
    in.close();

    fs::create_directories("data");
    ofstream out("data/users.txt");
    for (const string& l : lines) {
        out << l << "\n";
    }
    return true;
}

// AUTH & SECURITY: Lists registered accounts and roles for Admin inspection
void viewAllUsersAdmin() {
    ifstream in("data/users.txt");
    cout << "\n--- ADMIN: REGISTERED ACCOUNTS ---\n";
    cout << left << setw(16) << "Username" << setw(12) << "Role"
         << right << setw(12) << "Wallet" << "  " << left << "Profile\n";
    cout << string(62, '-') << "\n";

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string u, p, role, bal, bio, priv;
        getline(ss, u, '|');
        getline(ss, p, '|');
        getline(ss, role, '|');
        getline(ss, bal, '|');
        getline(ss, bio, '|');
        getline(ss, priv, '|');

        string shownBio = (priv == "1") ? "[Private]" : bio;
        cout << left << setw(16) << u << setw(12) << role
             << right << setw(12) << bal << "  " << left << shownBio << "\n";
    }
}