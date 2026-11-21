#ifndef AUTH_H // Prevents multiple inclusion of auth.h
#define AUTH_H // Marks AUTH_H as defined

#include <string> // Enables string usage in declarations

// AUTH & PROFILES: Stores active user session state, profile info, privacy setting, and wallet balance
struct Session {
    bool loggedIn = false;          // True once login + MFA succeeds
    std::string username;           // Logged-in account username
    std::string role;               // Role: Student, Tutor, Admin, or Engineer
    std::string profileBio;         // Short user profile summary/headline
    bool privateProfile = false;    // Privacy control flag (hides profile details when true)
    double walletBalance = 0.0;     // Balance used for tutor bookings/payments
};

// AUTH & SECURITY: Initializes data directory and default user/profile records
void initAuthFiles();

// AUTH & SECURITY: Registers a new account with password encoding, role selection, and profile info
bool registerUser();

// AUTH & SECURITY: Authenticates credentials and verifies a 6-digit MFA code written to data/mfa_inbox.txt
bool loginUser(Session& activeSession);

// AUTH & PROFILES: Views or edits user profile, privacy controls, and encryption status
void manageProfileAndPrivacy(Session& activeSession);

// AUTH & PROFILES: Helper to update a user's wallet balance in data/users.txt
bool updateWalletBalance(const std::string& username, double newBalance);

// AUTH & SECURITY: Displays user account summary for Admin role
void viewAllUsersAdmin();

#endif // End of AUTH_H