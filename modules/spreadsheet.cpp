#include "spreadsheet.hpp"   // Declares runSpreadsheetModule()
#include <cctype>    // Provides toupper and isdigit
#include <cstdlib>   // Provides atoi and strtod
#include <filesystem>// Provides filesystem directory creation
#include <fstream>   // Provides ifstream and ofstream
#include <iomanip>   // Provides setw, left, fixed, setprecision
#include <iostream>  // Provides cin and cout
#include <limits>    // Provides numeric_limits
#include <sstream>   // Provides stringstream and ostringstream
#include <string>    // Provides string helpers
#include <vector>    // Provides vector for 2D grid

using namespace std; // Brings std names into sheet.cpp
namespace fs = filesystem; // Short alias for std::filesystem

namespace { // Private spreadsheet helpers

constexpr int ROWS = 5; // SPREADSHEET: Grid row count (1..5)
constexpr int COLS = 5; // SPREADSHEET: Grid column count (A..E)

// SPREADSHEET: Extracts a numeric value from plain numbers or currency-formatted strings like "$15.00"
bool tryParseNumericCell(const string& raw, double& outVal) {
    string cleaned;
    for (unsigned char ch : raw) {
        if (isdigit(ch) || ch == '.' || ch == '-' || ch == '+') {
            cleaned.push_back(static_cast<char>(ch));
        }
    }
    if (cleaned.empty() || cleaned == "-" || cleaned == "+" || cleaned == ".") {
        return false;
    }
    char* endPtr = nullptr;
    double num = strtod(cleaned.c_str(), &endPtr);
    if (endPtr != cleaned.c_str() && *endPtr == '\0') {
        outVal = num;
        return true;
    }
    return false;
}

// SPREADSHEET: Converts cell reference like "A1" into 0-based row and column indices
bool parseCellRef(const string& ref, int& r, int& c) {
    if (ref.size() < 2) return false;
    char colChar = static_cast<char>(toupper(static_cast<unsigned char>(ref[0])));
    if (colChar < 'A' || colChar >= 'A' + COLS) return false;

    string rowPart = ref.substr(1);
    for (char ch : rowPart) {
        if (!isdigit(static_cast<unsigned char>(ch))) return false;
    }

    int rowNum = atoi(rowPart.c_str());
    if (rowNum < 1 || rowNum > ROWS) return false;

    c = colChar - 'A';
    r = rowNum - 1;
    return true;
}

// SPREADSHEET: Evaluates a number or =SUM(A1:A5), =AVG(A1:A5), or =PROD(A1:A5)
double evalCellOrFormula(const string& raw,
                          const vector<vector<string>>& grid,
                          int depth = 0) {
    if (raw.empty() || depth > 8) return 0.0;

    if (raw[0] != '=') {
        double parsedNum = 0.0;
        if (tryParseNumericCell(raw, parsedNum)) {
            return parsedNum; // Direct or currency-formatted numeric value
        }
        return 0.0;
    }

    if (raw[0] == '=') {
        string upper = raw;
        for (char& ch : upper) ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));

        bool isSum = (upper.rfind("=SUM(", 0) == 0);
        bool isAvg = (upper.rfind("=AVG(", 0) == 0);
        bool isProd = (upper.rfind("=PROD(", 0) == 0);

        size_t openParen = upper.find('(');
        size_t closeParen = upper.find(')');

        if ((isSum || isAvg || isProd) && openParen != string::npos && closeParen != string::npos) {
            string inside = upper.substr(openParen + 1, closeParen - openParen - 1);
            size_t colon = inside.find(':');
            if (colon != string::npos) {
                int r1, c1, r2, c2;
                if (parseCellRef(inside.substr(0, colon), r1, c1) &&
                    parseCellRef(inside.substr(colon + 1), r2, c2)) {
                    if (r1 > r2) swap(r1, r2);
                    if (c1 > c2) swap(c1, c2);

                    double total = 0.0;
                    double product = 1.0;
                    int count = 0;
                    for (int r = r1; r <= r2; ++r) {
                        for (int c = c1; c <= c2; ++c) {
                            double v = evalCellOrFormula(grid[r][c], grid, depth + 1);
                            total += v;
                            product *= v;
                            ++count;
                        }
                    }
                    if (isSum) return total;
                    if (isAvg && count > 0) return total / count;
                    if (isProd && count > 0) return product;
                }
            }
        }
    }
    return 0.0;
}

// SPREADSHEET: Formats cell text for table output
string displayCell(const string& raw, const vector<vector<string>>& grid) {
    if (raw.empty()) return "";
    if (raw[0] == '=') {
        double val = evalCellOrFormula(raw, grid);
        ostringstream oss;
        oss << fixed << setprecision(2) << val;
        return oss.str();
    }
    return raw;
}

// SPREADSHEET: Saves grid contents to data/sheet_data.csv
void saveSheet(const vector<vector<string>>& grid) {
    fs::create_directories("data");
    ofstream out("data/sheet_data.csv");
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            out << grid[r][c];
            if (c < COLS - 1) out << ",";
        }
        out << "\n";
    }
    cout << "[SPREADSHEET] Saved to data/sheet_data.csv\n";
}

// SPREADSHEET: Loads grid contents from data/sheet_data.csv if present
void loadSheet(vector<vector<string>>& grid) {
    ifstream in("data/sheet_data.csv");
    if (!in.is_open()) return;
    string line;
    int r = 0;
    while (getline(in, line) && r < ROWS) {
        stringstream ss(line);
        string cell;
        int c = 0;
        while (getline(ss, cell, ',') && c < COLS) {
            grid[r][c] = cell;
            ++c;
        }
        ++r;
    }
}

// SPREADSHEET: Prints aligned ASCII grid using iomanip
void printGrid(const vector<vector<string>>& grid) {
    cout << "\n" << setw(5) << left << "" << "|";
    for (int c = 0; c < COLS; ++c) {
        string colLabel(1, static_cast<char>('A' + c));
        cout << " " << setw(12) << left << colLabel << "|";
    }
    cout << "\n";

    for (int r = 0; r < ROWS; ++r) {
        string rowLabel = "R" + to_string(r + 1);
        cout << setw(5) << left << rowLabel << "|";
        for (int c = 0; c < COLS; ++c) {
            string text = displayCell(grid[r][c], grid);
            if (text.size() > 12) text = text.substr(0, 12);
            cout << " " << setw(12) << left << text << "|";
        }
        cout << "\n";
    }
}

} // namespace

// SPREADSHEET: Main interactive loop
void runSpreadsheetModule() {
    vector<vector<string>> grid(ROWS, vector<string>(COLS, ""));
    loadSheet(grid);

    int choice = -1;
    while (choice != 0) {
        printGrid(grid);
        cout << "\n--- SPREADSHEET MENU ---\n";
        cout << "[1] Set Cell (number, text, =SUM(A1:A5), =AVG(A1:A5), =PROD(A1:A5))\n";
        cout << "[2] Clear Cell\n";
        cout << "[3] Save to CSV (data/sheet_data.csv)\n";
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
            string ref, val;
            int r, c;
            cout << "Cell (A1-E5): ";
            getline(cin, ref);
            if (!parseCellRef(ref, r, c)) {
                cout << "Invalid cell reference.\n";
                continue;
            }
            cout << "Value or Formula: ";
            getline(cin, val);
            if (val.find(',') != string::npos) {
                cout << "Commas are not supported in cells here.\n";
                continue;
            }
            grid[r][c] = val;
        } else if (choice == 2) {
            string ref;
            int r, c;
            cout << "Cell to clear (A1-E5): ";
            getline(cin, ref);
            if (parseCellRef(ref, r, c)) {
                grid[r][c].clear();
            } else {
                cout << "Invalid cell reference.\n";
            }
        } else if (choice == 3) {
            saveSheet(grid);
        }
    }
}