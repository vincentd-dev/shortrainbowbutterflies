#include "online_tutor.hpp" // Declares runFormulaAssistantModule()
#include <cctype>   // Provides tolower for case-insensitive keyword matching
#include <cstdio>   // Provides popen/pclose/remove for invoking curl and reading temp output
#include <fstream>  // Provides ofstream/ifstream for writing JSON payload and reading response
#include <iostream> // Provides cin and cout
#include <sstream>  // Provides ostringstream/istringstream for building JSON and parsing text
#include <string>   // Provides string

using namespace std; // Brings std names into online_tutor.cpp

namespace { // Private knowledge-base and local Ollama helpers

// AI INTEGRATION: Converts text to lowercase for simple keyword matching
string toLower(const string& s) {
    string out = s;
    for (char& ch : out) {
        ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

// AI INTEGRATION: Escapes special characters so user prompts form valid JSON strings
string escapeJson(const string& text) {
    ostringstream out;
    for (char ch : text) {
        switch (ch) {
            case '\"': out << "\\\""; break; // Escape double quote
            case '\\': out << "\\\\"; break; // Escape backslash
            case '\n': out << "\\n";  break; // Escape newline
            case '\r': out << "\\r";  break; // Escape carriage return
            case '\t': out << "\\t";  break; // Escape tab
            default:   out << ch;       break; // Keep normal characters as-is
        }
    }
    return out.str();
}

// AI INTEGRATION: Extracts assistant message content from Ollama /api/chat JSON response
string extractOllamaContent(const string& json) {
    // Look for "message" first so we read the reply rather than prompt/metadata fields
    size_t msgPos = json.find("\"message\"");
    size_t searchStart = (msgPos != string::npos) ? msgPos : 0;
    size_t contentKey = json.find("\"content\"", searchStart);
    if (contentKey == string::npos) return "";

    size_t colonPos = json.find(':', contentKey + 9);
    if (colonPos == string::npos) return "";

    size_t quoteStart = json.find('\"', colonPos + 1);
    if (quoteStart == string::npos) return "";

    // Walk character by character to respect escaped quotes and control escapes
    string result;
    bool escaped = false;
    for (size_t i = quoteStart + 1; i < json.size(); ++i) {
        char c = json[i];
        if (escaped) {
            switch (c) {
                case 'n':  result.push_back('\n'); break;
                case 'r':  result.push_back('\r'); break;
                case 't':  result.push_back('\t'); break;
                case '\"': result.push_back('\"'); break;
                case '\\': result.push_back('\\'); break;
                default:   result.push_back(c);    break;
            }
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '\"') {
            return result; // Reached closing quote of "content"
        } else {
            result.push_back(c);
        }
    }
    return "";
}

// AI INTEGRATION: Queries local Ollama server via curl; returns empty string if unavailable
string queryLocalOllama(const string& prompt, const string& model) {
    const string payloadFile = "engitool_ollama_req.json";
    const string responseFile = "engitool_ollama_res.json";

    // Write non-streaming /api/chat payload targeting local Ollama daemon
    ofstream reqOut(payloadFile, ios::binary);
    if (!reqOut.is_open()) return "";

    reqOut << "{"
           << "\"model\":\"" << escapeJson(model) << "\","
           << "\"stream\":false,"
           << "\"messages\":["
           << "{\"role\":\"system\",\"content\":\"You are a concise engineering and study tutor. Give clear formulas, steps, and explanations.\"},"
           << "{\"role\":\"user\",\"content\":\"" << escapeJson(prompt) << "\"}"
           << "]"
           << "}";
    reqOut.close();

    // Use curl.exe on Windows (_WIN32) and curl on Unix-like systems with short timeouts
#ifdef _WIN32
    string cmd = "curl.exe -sS --max-time 20 -X POST http://localhost:11434/api/chat "
                 "-H \"Content-Type: application/json\" --data-binary @" + payloadFile +
                 " > " + responseFile + " 2>NUL";
    FILE* pipe = _popen(cmd.c_str(), "r");
    int exitCode = -1;
    if (pipe != nullptr) {
        exitCode = _pclose(pipe);
    }
#else
    string cmd = "curl -sS --max-time 20 -X POST http://localhost:11434/api/chat "
                 "-H 'Content-Type: application/json' --data-binary @" + payloadFile +
                 " > " + responseFile + " 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    int exitCode = -1;
    if (pipe != nullptr) {
        exitCode = pclose(pipe);
    }
#endif

    string reply;
    if (exitCode == 0) {
        ifstream resIn(responseFile, ios::binary);
        if (resIn.is_open()) {
            ostringstream buffer;
            buffer << resIn.rdbuf();
            reply = extractOllamaContent(buffer.str());
        }
    }

    // Clean up temporary request/response files regardless of outcome
    remove(payloadFile.c_str());
    remove(responseFile.c_str());

    return reply;
}

// AI INTEGRATION: Matches user query against built-in engineering & study topics (offline fallback)
string answerPromptOffline(const string& prompt) {
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

// AI INTEGRATION: Interactive prompt loop trying local Ollama first, then offline rule-based fallback
void runFormulaAssistantModule(const string& model) {
    string input;
    cout << "\n--- ENGINEERING & STUDY ASSISTANT (LOCAL AI HELPER) ---\n";
    cout << "Model target: " << model << " (http://localhost:11434) | Offline fallback enabled\n";
    cout << "Ask a formula/topic question (type 'exit' to return):\n";
    while (true) {
        cout << "\nYou> ";
        getline(cin, input);
        string lowered = toLower(input);
        if (lowered == "exit" || lowered == "quit") break;
        if (input.empty()) continue;

        // Try live local Ollama first; if not running or unreachable, fall back to built-in rules
        string ollamaReply = queryLocalOllama(input, model);
        if (!ollamaReply.empty()) {
            cout << "AI (Ollama)> " << ollamaReply << "\n";
        } else {
            cout << "AI (Offline)> " << answerPromptOffline(input) << "\n";
        }
    }
}