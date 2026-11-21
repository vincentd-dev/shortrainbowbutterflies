#include "messaging_platform.hpp" // Declares workspace, messaging, and tutor functions
#include <cstdlib> // Provides strtod
#include <fstream> // Provides ifstream and ofstream
#include <iomanip> // Provides setw, left, fixed, setprecision
#include <iostream>// Provides cin and cout
#include <limits>  // Provides numeric_limits
#include <sstream> // Provides stringstream
#include <vector>  // Provides vector

using namespace std; // Brings std names into comms.cpp

namespace { // Private helpers for workspace, messaging, and online tutors

// TEAM COLLABORATION: Adds an entry to the shared workspace board
void addWorkspaceItem(const string& username) {
    string item;
    cout << "Shared workspace update/task: ";
    getline(cin, item);
    for (char& ch : item) if (ch == '|') ch = '/';

    ofstream out("data/workspace.txt", ios::app);
    if (out.is_open()) {
        out << username << "|" << item << "\n";
        cout << "[TEAM COLLABORATION] Posted to shared workspace.\n";
    }
}

// TEAM COLLABORATION: Views shared workspace entries
void viewWorkspace() {
    ifstream in("data/workspace.txt");
    cout << "\n--- SHARED TEAM WORKSPACE ---\n";
    if (!in.is_open()) {
        cout << "No workspace entries yet.\n";
        return;
    }
    string line;
    int n = 1;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string user, item;
        getline(ss, user, '|');
        getline(ss, item, '|');
        cout << n++ << ". [" << user << "] " << item << "\n";
    }
}

// MESSAGING PLATFORM: Sends a direct message or group chat message
void sendMessage(const string& sender) {
    string channelType, target, msg;
    cout << "Type (Direct / Group): ";
    getline(cin, channelType);
    cout << "Target user or group name (e.g., tutor1 or TeamAlpha): ";
    getline(cin, target);
    cout << "Message: ";
    getline(cin, msg);

    for (char& ch : channelType) if (ch == '|') ch = '/';
    for (char& ch : target) if (ch == '|') ch = '/';
    for (char& ch : msg) if (ch == '|') ch = '/';

    ofstream out("data/messages.txt", ios::app);
    if (out.is_open()) {
        out << channelType << "|" << sender << "|" << target << "|" << msg << "\n";
        cout << "[MESSAGING] Message sent to '" << target << "'.\n";
    }
}

// MESSAGING PLATFORM: Displays direct and group chat history
void viewMessages() {
    ifstream in("data/messages.txt");
    cout << "\n--- DIRECT & GROUP MESSAGES ---\n";
    if (!in.is_open()) {
        cout << "No messages yet.\n";
        return;
    }
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string type, sender, target, msg;
        getline(ss, type, '|');
        getline(ss, sender, '|');
        getline(ss, target, '|');
        getline(ss, msg, '|');
        cout << "[" << type << "] " << sender << " -> " << target << ": " << msg << "\n";
    }
}

struct TutorProfile {
    string id;
    string name;
    string specialty;
    string verificationStatus;
    string scheduleSlot;
    double hourlyRate = 0.0;
    double ratingSum = 0.0;
    int ratingCount = 0;
};

// ONLINE TUTOR: Seeds sample tutor profiles if data/tutors.txt does not exist
void ensureTutorSeedFile() {
    ifstream check("data/tutors.txt");
    if (check.good()) return;

    ofstream out("data/tutors.txt");
    out << "T1|Eng. Cruz|Calculus & Statics|CV Verified, Certified, BG Checked|Mon/Wed 4PM|150.00|14.4|3\n";
    out << "T2|Prof. Reyes|Circuits & Python|CV Verified, Certified, BG Checked|Tue/Thu 5PM|180.00|9.6|2\n";
}

// ONLINE TUTOR: Loads tutors from data/tutors.txt
vector<TutorProfile> loadTutors() {
    ensureTutorSeedFile();
    vector<TutorProfile> list;
    ifstream in("data/tutors.txt");
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        TutorProfile t;
        string rateStr, sumStr, cntStr;
        getline(ss, t.id, '|');
        getline(ss, t.name, '|');
        getline(ss, t.specialty, '|');
        getline(ss, t.verificationStatus, '|');
        getline(ss, t.scheduleSlot, '|');
        getline(ss, rateStr, '|');
        getline(ss, sumStr, '|');
        getline(ss, cntStr, '|');
        t.hourlyRate = strtod(rateStr.c_str(), nullptr);
        t.ratingSum = strtod(sumStr.c_str(), nullptr);
        t.ratingCount = static_cast<int>(strtod(cntStr.c_str(), nullptr));
        list.push_back(t);
    }
    return list;
}

// ONLINE TUTOR: Saves updated tutor profiles/ratings back to data/tutors.txt
void saveTutors(const vector<TutorProfile>& list) {
    ofstream out("data/tutors.txt");
    for (const auto& t : list) {
        out << t.id << "|" << t.name << "|" << t.specialty << "|"
            << t.verificationStatus << "|" << t.scheduleSlot << "|"
            << fixed << setprecision(2) << t.hourlyRate << "|"
            << t.ratingSum << "|" << t.ratingCount << "\n";
    }
}

// ONLINE TUTOR: Displays tutor profiles, verification badges, ratings, and rates
void showTutors(const vector<TutorProfile>& list) {
    cout << "\n--- ONLINE TUTOR DIRECTORY ---\n";
    for (const auto& t : list) {
        double avgRating = (t.ratingCount > 0) ? (t.ratingSum / t.ratingCount) : 0.0;
        cout << "[" << t.id << "] " << t.name << " | Specialty: " << t.specialty
             << " | Rate: PHP " << fixed << setprecision(2) << t.hourlyRate << "/hr"
             << " | Slot: " << t.scheduleSlot
             << " | Rating: " << setprecision(1) << avgRating << "/5.0 (" << t.ratingCount << ")\n";
        cout << "     Verification & Badges: " << t.verificationStatus << "\n";
    }
}

} // namespace

// TEAM COLLABORATION & MESSAGING: Interactive menu for shared workspace and messages
void runWorkspaceAndMessagesModule(const string& username) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- TEAM COLLABORATION & MESSAGING ---\n";
        cout << "[1] Post to Shared Team Workspace\n";
        cout << "[2] View Shared Team Workspace\n";
        cout << "[3] Send Direct / Group Message\n";
        cout << "[4] View Messages\n";
        cout << "[0] Back to Dashboard\n";
        cout << "Choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: addWorkspaceItem(username); break;
            case 2: viewWorkspace(); break;
            case 3: sendMessage(username); break;
            case 4: viewMessages(); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; break;
        }
    }
}

// ONLINE TUTOR: Interactive menu for skill assessment, booking, payment, and ratings
void runOnlineTutorModule(Session& activeSession) {
    vector<TutorProfile> tutors = loadTutors();
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- ONLINE TUTOR PLATFORM ---\n";
        cout << "[1] Browse Tutors (Profiles, Verifications, Ratings & Slots)\n";
        cout << "[2] Quick Skill Self-Assessment Check\n";
        cout << "[3] Book Tutor Slot & Pay via Simulated Wallet\n";
        cout << "[4] Rate & Review a Tutor\n";
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
            showTutors(tutors);
        } else if (choice == 2) {
            int ans = 0;
            cout << "\n[Skill Assessment] What is the derivative of x^2 with respect to x?\n";
            cout << "[1] x   [2] 2x   [3] x^3\nAnswer: ";
            cin >> ans;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (ans == 2) {
                cout << "Correct! Recommended focus: Calculus / Differential Equations.\n";
            } else {
                cout << "Hint: Bring down the exponent (2x). Recommended tutor: T1 (Eng. Cruz).\n";
            }
        } else if (choice == 3) {
            showTutors(tutors);
            string id;
            cout << "Enter Tutor ID to book (e.g., T1): ";
            getline(cin, id);
            bool booked = false;
            for (const auto& t : tutors) {
                if (t.id == id) {
                    if (activeSession.walletBalance < t.hourlyRate) {
                        cout << "[PAYMENT] Insufficient wallet balance (PHP " << activeSession.walletBalance
                             << " < PHP " << t.hourlyRate << ").\n";
                    } else {
                        activeSession.walletBalance -= t.hourlyRate;
                        updateWalletBalance(activeSession.username, activeSession.walletBalance);
                        ofstream log("data/tutor_bookings.txt", ios::app);
                        log << activeSession.username << "|" << t.id << "|" << t.name
                            << "|" << t.scheduleSlot << "|PAID:" << t.hourlyRate << "\n";
                        cout << "[BOOKING & PAYMENT] Booked " << t.name << " (" << t.scheduleSlot
                             << "). Paid PHP " << fixed << setprecision(2) << t.hourlyRate << ".\n";
                    }
                    booked = true;
                    break;
                }
            }
            if (!booked) cout << "Tutor ID not found.\n";
        } else if (choice == 4) {
            showTutors(tutors);
            string id;
            double score = 0.0;
            cout << "Enter Tutor ID to rate: ";
            getline(cin, id);
            cout << "Rating (1.0 to 5.0): ";
            cin >> score;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (score >= 1.0 && score <= 5.0) {
                for (auto& t : tutors) {
                    if (t.id == id) {
                        t.ratingSum += score;
                        t.ratingCount += 1;
                        saveTutors(tutors);
                        cout << "[RATINGS] Thank you! Rating recorded.\n";
                    }
                }
            } else {
                cout << "Rating must be between 1.0 and 5.0.\n";
            }
        }
    }
}
