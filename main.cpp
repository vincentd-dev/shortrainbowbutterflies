#include "modules/user_accounts_security.hpp"    // Auth, MFA, user profiles, privacy, and admin view
#include "modules/advancedcalculator.hpp"    // ENGINEERING CALCULATOR
#include "modules/messaging_platform.hpp"   // TEAM COLLABORATION, MESSAGING & ONLINE TUTOR
#include "modules/online_tutor.hpp"  // AI INTEGRATION (Formula & study assistant)
#include "modules/schedule_planner.hpp" // OFFLINE MODE, NOTES & SCHEDULE PLANNER
#include "modules/spreadsheet.hpp"   // SPREADSHEET
#include "modules/cloud_storage.hpp"   // CLOUD STORAGE
#include <iomanip>           // Provides fixed, setprecision, setw, left
#include <iostream>          // Provides cin and cout
#include <limits>            // Provides numeric_limits

using namespace std; // Brings std names into main.cpp

namespace { // File-local dashboard helpers

// Clears stream error flags and discards leftover characters until newline
void clearBadInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Displays main dashboard and routes every feature menu option
void showDashboard(Session& session) {
    int option = -1;
    while (option != 0) {
        cout << "\n" << string(46, '=') << "\n";
        cout << " ENGITOOL SUITE | User: " << session.username
             << " | Role: " << session.role << "\n";
        cout << " Wallet: PHP " << fixed << setprecision(2)
             << session.walletBalance << "\n";
        cout << string(46, '=') << "\n";
        cout << left << setw(5) << "[1]" << "Advanced Engineering Calculator\n";
        cout << left << setw(5) << "[2]" << "Mini Spreadsheet (Formulas & CSV)\n";
        cout << left << setw(5) << "[3]" << "Offline Notes & Schedule Planner\n";
        cout << left << setw(5) << "[4]" << "Cloud Storage (Backups, History, Sync)\n";
        cout << left << setw(5) << "[5]" << "Team Workspace & Direct/Group Messages\n";
        cout << left << setw(5) << "[6]" << "Online Tutor (Profiles, Booking, Pay, Rate)\n";
        cout << left << setw(5) << "[7]" << "AI Integration (Engineering & Study Helper)\n";
        cout << left << setw(5) << "[8]" << "User Profile, Privacy & Wallet\n";
        if (session.role == "Admin") {
            cout << left << setw(5) << "[9]" << "Admin Panel (View Users)\n";
        }
        cout << left << setw(5) << "[0]" << "Logout\n";
        cout << "Choice: ";

        if (!(cin >> option)) {
            clearBadInput();
            cout << "Please enter a valid number.\n";
            option = -1;
            continue;
        }
        clearBadInput();

        switch (option) {
            case 1: runEngineeringCalculator(); break;
            case 2: runSpreadsheetModule(); break;
            case 3: runPlannerAndNotesModule(session.username); break;
            case 4: runCloudStorageModule(session.username); break;
            case 5: runWorkspaceAndMessagesModule(session.username); break;
            case 6: runOnlineTutorModule(session); break;
            case 7: runFormulaAssistantModule(); break;
            case 8: manageProfileAndPrivacy(session); break;
            case 9:
                if (session.role == "Admin") {
                    viewAllUsersAdmin();
                } else {
                    cout << "Access denied: Admin role required.\n";
                }
                break;
            case 0:
                session.loggedIn = false;
                session.username.clear();
                session.role.clear();
                session.walletBalance = 0.0;
                cout << "Logged out.\n";
                break;
            default:
                cout << "Unknown option.\n";
                break;
        }
    }
}

} // namespace

// Entry point: initializes files and runs Sign Up / Login / Exit menu
int main() {
    initAuthFiles();

    int choice = -1;
    while (choice != 0) {
        cout << "\n" << string(32, '=') << "\n";
        cout << "     ENGITOOL TERMINAL SUITE    \n";
        cout << string(32, '=') << "\n";
        cout << "[1] Login (with MFA)\n";
        cout << "[2] Sign Up\n";
        cout << "[0] Exit\n";
        cout << "Choice: ";

        if (!(cin >> choice)) {
            clearBadInput();
            cout << "Invalid input.\n";
            choice = -1;
            continue;
        }
        clearBadInput();

        if (choice == 1) {
            Session currentSession;
            if (loginUser(currentSession)) {
                showDashboard(currentSession);
            }
        } else if (choice == 2) {
            registerUser();
        } else if (choice == 0) {
            cout << "Exiting Engitool. Goodbye!\n";
        } else {
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}