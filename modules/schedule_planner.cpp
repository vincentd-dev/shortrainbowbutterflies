#include "schedule_planner.hpp" // Declares runPlannerAndNotesModule()
#include <filesystem> // Provides filesystem directory helpers
#include <fstream>   // Provides ifstream and ofstream for offline persistence
#include <iomanip>   // Provides setw and left for formatted lists
#include <iostream>  // Provides cin and cout
#include <limits>    // Provides numeric_limits
#include <sstream>   // Provides stringstream

using namespace std; // Brings std names into planner.cpp
namespace fs = filesystem;

namespace { // Private helpers for notes and schedule planning

// NOTES & OFFLINE MODE: Adds an offline note to data/notes.txt
void addNote(const string& username) {
    string title, body;
    cout << "Note title: ";
    getline(cin, title);
    cout << "Note content: ";
    getline(cin, body);
    if (title.empty() || body.empty()) {
        cout << "[OFFLINE NOTES] Title and content cannot be empty.\n";
        return;
    }
    for (char& ch : title) if (ch == '|') ch = '/';
    for (char& ch : body) if (ch == '|') ch = '/';

    fs::create_directories("data");
    ofstream out("data/notes.txt", ios::app);
    if (out.is_open()) {
        out << username << "|" << title << "|" << body << "\n";
        cout << "[OFFLINE NOTES] Note saved locally.\n";
    }
}

// NOTES & OFFLINE MODE: Displays saved notes from data/notes.txt
void viewNotes() {
    ifstream in("data/notes.txt");
    cout << "\n--- OFFLINE NOTES ---\n";
    if (!in.is_open()) {
        cout << "No notes saved yet.\n";
        return;
    }
    string line;
    int idx = 1;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string user, title, body;
        getline(ss, user, '|');
        getline(ss, title, '|');
        getline(ss, body, '|');
        cout << idx++ << ". [" << title << "] (by " << user << "): " << body << "\n";
    }
}

// SCHEDULE PLANNER: Adds a calendar event, task, study schedule item, or project deadline
void addScheduleItem(const string& username) {
    string kind, title, dateStr;
    cout << "Type (Schedule / Task / Deadline): ";
    getline(cin, kind);
    cout << "Title: ";
    getline(cin, title);
    cout << "Date (e.g., 2026-12-01): ";
    getline(cin, dateStr);

    if (title.empty() || dateStr.empty()) {
        cout << "[SCHEDULE PLANNER] Title and date cannot be empty.\n";
        return;
    }
    if (kind.empty()) {
        kind = "Task";
    }

    for (char& ch : kind) if (ch == '|') ch = '/';
    for (char& ch : title) if (ch == '|') ch = '/';
    for (char& ch : dateStr) if (ch == '|') ch = '/';

    fs::create_directories("data");
    ofstream out("data/schedule.txt", ios::app);
    if (out.is_open()) {
        out << username << "|" << kind << "|" << dateStr << "|" << title << "\n";
        cout << "[SCHEDULE PLANNER] Added to local schedule.\n";
    }
}

// SCHEDULE PLANNER: Displays scheduled events, tasks, study sessions, and deadlines
void viewSchedule() {
    ifstream in("data/schedule.txt");
    cout << "\n--- CALENDAR, TASKS, STUDY SCHEDULE & DEADLINES ---\n";
    if (!in.is_open()) {
        cout << "No schedule items yet.\n";
        return;
    }
    cout << left << setw(14) << "Date" << setw(14) << "Type"
         << setw(16) << "Owner" << "Title\n";
    cout << string(66, '-') << "\n";

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string user, kind, dateStr, title;
        getline(ss, user, '|');
        getline(ss, kind, '|');
        getline(ss, dateStr, '|');
        getline(ss, title, '|');
        cout << left << setw(14) << dateStr << setw(14) << kind
             << setw(16) << user << title << "\n";
    }
}

} // namespace

// OFFLINE MODE, NOTES & SCHEDULE PLANNER: Interactive menu loop
void runPlannerAndNotesModule(const string& username) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- OFFLINE NOTES & SCHEDULE PLANNER (OFFLINE MODE READY) ---\n";
        cout << "[1] Create Offline Note\n";
        cout << "[2] View Offline Notes\n";
        cout << "[3] Add Calendar / Task / Study / Deadline Item\n";
        cout << "[4] View Schedule Planner\n";
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
            case 1: addNote(username); break;
            case 2: viewNotes(); break;
            case 3: addScheduleItem(username); break;
            case 4: viewSchedule(); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; break;
        }
    }
}