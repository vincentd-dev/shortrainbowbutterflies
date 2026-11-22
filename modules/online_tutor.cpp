#include "online_tutor.hpp" // Declares runFormulaAssistantModule()
#include <algorithm> // Provides transform
#include <cctype>    // Provides tolower and isspace
#include <cstdio>    // Provides remove for cleaning up temporary request/response files
#include <fstream>   // Provides ofstream/ifstream for writing JSON payload and reading response
#include <iostream>  // Provides cin and cout
#include <sstream>   // Provides ostringstream for building JSON and reading response text
#include <string>    // Provides string
#include <vector>    // Provides vector for mutable command buffer

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // Provides CreateProcessA, WaitForSingleObject, and SetConsoleOutputCP on Windows
#endif

using namespace std; // Brings std names into online_tutor.cpp

namespace { // Private knowledge-base and local Ollama helpers

// AI INTEGRATION: Ensures Windows console outputs UTF-8 special characters properly
void enableUtf8ConsoleOutput() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

// AI INTEGRATION: Converts text to lowercase for simple keyword matching
string toLower(const string& s) {
    string out = s;
    for (char& ch : out) {
        ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

// AI INTEGRATION: Trims leading and trailing ASCII whitespace
string trimCopy(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

// AI INTEGRATION: Escapes special characters so user prompts form valid JSON strings
string escapeJson(const string& text) {
    ostringstream out;
    for (char ch : text) {
        switch (ch) {
            case '\"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;
            default:   out << ch;       break;
        }
    }
    return out.str();
}

// AI INTEGRATION: Parses 4 hex digits into an unsigned code point
bool parseHex4(const string& s, size_t pos, unsigned int& outVal) {
    if (pos + 4 > s.size()) return false;
    unsigned int val = 0;
    for (size_t i = 0; i < 4; ++i) {
        char c = s[pos + i];
        val <<= 4;
        if (c >= '0' && c <= '9') {
            val |= static_cast<unsigned int>(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            val |= static_cast<unsigned int>(10 + (c - 'a'));
        } else if (c >= 'A' && c <= 'F') {
            val |= static_cast<unsigned int>(10 + (c - 'A'));
        } else {
            return false;
        }
    }
    outVal = val;
    return true;
}

// AI INTEGRATION: Encodes a Basic Multilingual Plane code point (special math/symbol chars) as UTF-8
void appendUtf8SpecialChar(string& out, unsigned int cp) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF && !(cp >= 0xD800 && cp <= 0xDFFF)) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

// AI INTEGRATION: Normalizes fraction characters so fractions stay clear in terminal output
string normalizeFractionsForTerminal(const string& text) {
    string out;
    out.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        // UTF-8 Latin-1 Supplement vulgar fractions (0xC2 0xBC..0xBE)
        if (c == 0xC2 && i + 1 < text.size()) {
            unsigned char next = static_cast<unsigned char>(text[i + 1]);
            if (next == 0xBC) { out += "(1/4)"; ++i; continue; }
            if (next == 0xBD) { out += "(1/2)"; ++i; continue; }
            if (next == 0xBE) { out += "(3/4)"; ++i; continue; }
        }

        // UTF-8 General Punctuation fraction slash U+2044 (0xE2 0x81 0x84)
        if (c == 0xE2 && i + 2 < text.size() &&
            static_cast<unsigned char>(text[i + 1]) == 0x81 &&
            static_cast<unsigned char>(text[i + 2]) == 0x84) {
            out.push_back('/');
            i += 2;
            continue;
        }

        // UTF-8 Number Forms vulgar fractions U+2153..U+215E (0xE2 0x85 0x93..0x9E)
        if (c == 0xE2 && i + 2 < text.size() &&
            static_cast<unsigned char>(text[i + 1]) == 0x85) {
            unsigned char third = static_cast<unsigned char>(text[i + 2]);
            switch (third) {
                case 0x93: out += "(1/3)"; i += 2; continue;
                case 0x94: out += "(2/3)"; i += 2; continue;
                case 0x95: out += "(1/5)"; i += 2; continue;
                case 0x96: out += "(2/5)"; i += 2; continue;
                case 0x97: out += "(3/5)"; i += 2; continue;
                case 0x98: out += "(4/5)"; i += 2; continue;
                case 0x99: out += "(1/6)"; i += 2; continue;
                case 0x9A: out += "(5/6)"; i += 2; continue;
                case 0x9B: out += "(1/8)"; i += 2; continue;
                case 0x9C: out += "(3/8)"; i += 2; continue;
                case 0x9D: out += "(5/8)"; i += 2; continue;
                case 0x9E: out += "(7/8)"; i += 2; continue;
                default: break;
            }
        }

        out.push_back(static_cast<char>(c));
    }

    return out;
}

// AI INTEGRATION: Builds a 3-line stacked fraction block for terminal display
string makeStackedFraction(const string& numerator, const string& denominator) {
    size_t width = numerator.size() > denominator.size() ? numerator.size() : denominator.size();
    if (width < 1) width = 1;

    string bar(width, '-');
    string numPad((width - numerator.size()) / 2, ' ');
    string denPad((width - denominator.size()) / 2, ' ');

    ostringstream out;
    out << "\n"
        << numPad << numerator << "\n"
        << bar << "\n"
        << denPad << denominator << "\n";
    return out.str();
}

// AI INTEGRATION: Extracts assistant message content and decodes special character escapes
string extractOllamaContent(const string& json) {
    size_t msgPos = json.find("\"message\"");
    size_t searchStart = (msgPos != string::npos) ? msgPos : 0;
    size_t contentKey = json.find("\"content\"", searchStart);
    if (contentKey == string::npos) return "";

    size_t colonPos = json.find(':', contentKey + 9);
    if (colonPos == string::npos) return "";

    size_t quoteStart = json.find('\"', colonPos + 1);
    if (quoteStart == string::npos) return "";

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
                case '/':  result.push_back('/');  break;
                case 'u': {
                    unsigned int code = 0;
                    if (parseHex4(json, i + 1, code)) {
                        appendUtf8SpecialChar(result, code);
                        i += 4;
                    }
                    break;
                }
                default:
                    result.push_back(c);
                    break;
            }
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '\"') {
            return normalizeFractionsForTerminal(result);
        } else {
            result.push_back(c);
        }
    }
    return "";
}

// AI INTEGRATION: Runs curl directly without shell redirection issues; returns true on exit code 0
bool runCurlRequest(const string& payloadFile, const string& responseFile) {
#ifdef _WIN32
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hOut = CreateFileA(
        responseFile.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hOut == INVALID_HANDLE_VALUE) return false;

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = hOut;
    si.hStdError = hOut;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi{};
    string args = "curl.exe -4 -sS --connect-timeout 5 --max-time 300 -X POST "
                  "-H \"Content-Type: application/json\" --data-binary @" +
                  payloadFile + " http://127.0.0.1:11434/api/chat";

    vector<char> cmdBuf(args.begin(), args.end());
    cmdBuf.push_back('\0');

    BOOL ok = CreateProcessA(
        nullptr,
        cmdBuf.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    CloseHandle(hOut);
    if (!ok) return false;

    WaitForSingleObject(pi.hProcess, 310000);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode == 0;
#else
    string cmd = "curl -4 -sS --connect-timeout 5 --max-time 300 -X POST "
                 "-H 'Content-Type: application/json' --data-binary @" +
                 payloadFile + " http://127.0.0.1:11434/api/chat > " + responseFile;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe == nullptr) return false;
    int status = pclose(pipe);
    return status == 0;
#endif
}

// AI INTEGRATION: Builds system instruction based on active fraction layout mode
string buildSystemPrompt(bool stackedFractions) {
    string base =
        "You are a concise engineering and study tutor. Give clear formulas, steps, and brief "
        "explanations using standard math symbols such as ², ³, ×, ÷, ±, ≈, and →. ";

    if (stackedFractions) {
        return base +
               "Always present fractional formulas stacked on separate lines with a dashed bar '-' "
               "between the numerator on top and the denominator below, for example:\n"
               "f'(x)\n"
               "-----\n"
               "g'(x)\n"
               "Keep each stacked fraction on its own lines rather than inline '/' when presenting a formula.";
    }

    return base +
           "Write fractions clearly on a single line using parentheses and '/', such as (a + b) / c.";
}

// AI INTEGRATION: Queries local Ollama server via curl; returns empty string if unavailable
string queryLocalOllama(const string& prompt, const string& model, bool stackedFractions) {
    const string payloadFile = "engitool_ollama_req.json";
    const string responseFile = "engitool_ollama_res.json";

    ofstream reqOut(payloadFile, ios::binary);
    if (!reqOut.is_open()) return "";

    string systemPrompt = buildSystemPrompt(stackedFractions);

    reqOut << "{"
           << "\"model\":\"" << escapeJson(model) << "\","
           << "\"stream\":false,"
           << "\"options\":{\"num_predict\":280},"
           << "\"messages\":["
           << "{\"role\":\"system\",\"content\":\"" << escapeJson(systemPrompt) << "\"},"
           << "{\"role\":\"user\",\"content\":\"" << escapeJson(prompt) << "\"}"
           << "]"
           << "}";
    reqOut.close();

    string reply;
    if (runCurlRequest(payloadFile, responseFile)) {
        ifstream resIn(responseFile, ios::binary);
        if (resIn.is_open()) {
            ostringstream buffer;
            buffer << resIn.rdbuf();
            reply = extractOllamaContent(buffer.str());
        }
    }

    remove(payloadFile.c_str());
    remove(responseFile.c_str());

    return reply;
}

// AI INTEGRATION: Matches user query against built-in engineering & study topics (offline fallback)
string answerPromptOffline(const string& prompt, bool stackedFractions) {
    string q = toLower(prompt);
    if (q.find("hopital") != string::npos || q.find("l'hopital") != string::npos || q.find("limit") != string::npos) {
        if (stackedFractions) {
            return "L'Hopital's Rule (for 0/0 or ∞/∞): original vs derivative ratio" +
                   makeStackedFraction("f'(x)", "g'(x)");
        }
        return "L'Hopital's Rule: lim (f(x) / g(x)) = lim (f'(x) / g'(x)) for indeterminate 0/0 or ∞/∞ forms.";
    }
    if (q.find("ohm") != string::npos || q.find("voltage") != string::npos || q.find("current") != string::npos) {
        return "Ohm's Law: V = I × R (Voltage = Current × Resistance; Power P = V × I; I = V / R).";
    }
    if (q.find("stress") != string::npos || q.find("strain") != string::npos) {
        return "Normal Stress: σ = F / A (Force / Area); Strain: ε = ΔL / L0.";
    }
    if (q.find("velocity") != string::npos || q.find("kinematic") != string::npos || q.find("accel") != string::npos) {
        return "Kinematics: v = u + a·t, s = u·t + (1/2)·a·t², v² = u² + 2·a·s.";
    }
    if (q.find("pythag") != string::npos || q.find("triangle") != string::npos || q.find("right") != string::npos) {
        return "Pythagorean Theorem: a² + b² = c² (c = hypotenuse of a right triangle).";
    }
    if (q.find("study") != string::npos || q.find("plan") != string::npos || q.find("exam") != string::npos) {
        return "Study Tip: Break topics into 25 to 50 min focus blocks, solve practice problems, and review mistakes.";
    }
    return "Assistant Tip: Try keywords like 'l'hopital', 'ohm', 'stress', 'velocity', 'pythagorean', or 'study'.";
}

} // namespace

// AI INTEGRATION: Interactive prompt loop trying local Ollama first, then offline rule-based fallback
void runFormulaAssistantModule(const string& model) {
    enableUtf8ConsoleOutput();
    bool stackedFractions = true;
    string input;

    cout << "\n--- ENGINEERING & STUDY ASSISTANT (LOCAL AI HELPER) ---\n";
    cout << "Model target: " << model << " (http://127.0.0.1:11434) | Offline fallback enabled\n";
    cout << "Default fraction style: STACKED lines (type 'single line' or 'revert' to switch, 'stacked' to restore, 'exit' to return):\n";

    while (true) {
        cout << "\nYou> ";
        getline(cin, input);
        string trimmed = trimCopy(input);
        string lowered = toLower(trimmed);

        if (lowered == "exit" || lowered == "quit") break;
        if (trimmed.empty()) continue;

        if (lowered == "revert" || lowered == "single line" || lowered == "single-line" ||
            lowered == "inline" || lowered == "original") {
            stackedFractions = false;
            cout << "Formula format switched to single line (e.g., (a + b) / c). Type 'stacked' to return to stacked lines.\n";
            continue;
        }

        if (lowered == "stacked" || lowered == "stack" || lowered == "default") {
            stackedFractions = true;
            cout << "Formula format switched back to stacked lines.\n";
            continue;
        }

        cout << "Thinking...\n";
        string ollamaReply = queryLocalOllama(trimmed, model, stackedFractions);
        if (!ollamaReply.empty()) {
            cout << "AI (Ollama)> " << ollamaReply << "\n";
        } else {
            cout << "AI (Offline)> " << answerPromptOffline(trimmed, stackedFractions) << "\n";
        }
    }
}