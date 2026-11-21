#ifndef COMMS_H // Prevents multiple inclusion of comms.h
#define COMMS_H // Marks COMMS_H as defined

#include "user_accounts_security.hpp" // Provides Session struct for wallet updates
#include <string> // Enables string usage

using namespace std; // Enables standard library usage in header

// TEAM COLLABORATION & MESSAGING: Shared team workspace and direct/group messaging
void runWorkspaceAndMessagesModule(const string& username);

// ONLINE TUTOR: Tutor profiles, verifications, skill ratings, slot booking, and simulated payment
void runOnlineTutorModule(Session& activeSession);

#endif // End of COMMS_H
