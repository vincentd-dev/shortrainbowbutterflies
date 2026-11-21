#include "helper.h" // Declares runFormulaAssistantModule()
#include <cctype>   // Provides tolower for case-insensitive keyword matching
#include <iostream> // Provides cin and cout
#include <string>   // Provides string

using namespace std; // Brings std names into helper.cpp

namespace { // Private knowledge-base helpers

// AI INTEGRATION: Converts text to lowercase for simple keyword matching
string toLower(const string& s) {
    string out = s;
    for (char& ch : out) {
        ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

// AI INTEGRATION: Matches user query against built-in engineering & study topics
string answerPrompt(const string& prompt) {
    string q = toLower(prompt);
    if (q.find("ohm") != string::npos || q.find("voltage") != string::npos || q.find("current") != string::npos) {
        return "Ohm's Law: V = I * R (Voltage = Current * Resistance; Power P = V * I).";
    }
    if (q.find("stress") != string::npos || q.find("strain") != string::npos) {
        return "Normal Stress: sigma = F / A (Force / Area); Strain: epsilon = delta_L / L0.";
    }
    if (q.find("velocity") != string::npos || q.find("kinematic") != string::npos || q.find("accel") != string::npos) {
        return "Kinematics: v = u + a*t, s = u*t + 0.5*a*t^2, v^2 = u^2 + 2*a*s.";
    }
    if (q.find("pythag") != string::npos || q.find("triangle") != string::npos || q.find("right") != string::npos) {
        return "Pythagorean Theorem: a^2 + b^2 = c^2 (c = hypotenuse of a right triangle).";
    }
    if (q.find("study") != string::npos || q.find("plan") != string::npos || q.find("exam") != string::npos) {
        return "Study Tip: Break topics into 25-50 min focus blocks, solve practice problems, and review mistakes.";
    }
    return "Assistant Tip: Try keywords like 'ohm', 'stress', 'velocity', 'pythagorean', or 'study'.";
}

} // namespace

// AI INTEGRATION: Interactive prompt loop for formula and study help
void runFormulaAssistantModule() {
    string input;
    cout << "\n--- ENGINEERING & STUDY ASSISTANT (LOCAL AI HELPER) ---\n";
    cout << "Ask a formula/topic question (type 'exit' to return):\n";
    while (true) {
        cout << "\nYou> ";
        getline(cin, input);
        if (input == "exit" || input == "EXIT") break;
        if (input.empty()) continue;
        cout << "AI>  " << answerPrompt(input) << "\n";
    }
}
