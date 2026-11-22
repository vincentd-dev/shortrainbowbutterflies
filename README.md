# Engitool

C++17 terminal suite built with standard C++ and local text/CSV storage.

## Why It Is Split Into Modules

Keeping everything in one file gets messy fast once menus, math routines, file saves, and user checks pile up. Breaking the project into `main.cpp` and separate `.hpp` / `.cpp` files under `modules/` keeps each part easier to work on:

* `main.cpp` stays focused on startup, session handling, and routing between menus.
* Each feature lives in its own `.hpp` and `.cpp` pair so changes in one area do not accidentally break unrelated code.
* Headers (`.hpp`) expose the structs and functions needed across files, while `.cpp` files hold the implementation details.
* Adding or tweaking features later is much simpler when each module has a clear job.

## What `.hpp` Does and Why This Is Pure C++

* **What `.hpp` does:** A `.hpp` file is a C++ header file. Think of it like a list of declarations (functions, structs, and shared types) that tells other files what exists in a module, while the matching `.cpp` file holds the actual code that runs those features.
* **Why it is still pure C++:** Using `.hpp` instead of `.h` is just a naming convention for C++ header files. The core project is standard C++17 with no external web languages or heavy frameworks required, and optional local AI calls use `curl` against a local Ollama server.

## Detailed Module Breakdown

1. **`main.cpp` — Entry Point & Menu Router**
   * Acts as the main entry point of the application and connects all the modules together via `#include` headers.
   * Initializes local data storage and handles the login or sign up flow before opening the main workspace.
   * Routes the user into each feature module based on their menu selection and manages session logout or clean exit.

2. **`modules/user_accounts_security.hpp` and `.cpp` — User Accounts & Security**
   * Handles account registration, sign in, and active user session state.
   * Generates and checks 6 digit MFA verification codes during login.
   * Manages user profiles, roles, security/privacy preferences, and wallet balance updates saved to disk.

3. **`modules/advancedcalculator.hpp` and `.cpp` — Advanced Engineering Calculator**
   * Covers unit conversions for engineering and everyday quantities.
   * Implements trigonometric functions, basic statistical calculations, 2x2 matrix operations, and equation or complex number routines.

4. **`modules/spreadsheet.hpp` and `.cpp` — Spreadsheets**
   * Provides an interactive grid where cells can be viewed and edited directly in the terminal.
   * Evaluates formulas such as `SUM`, `AVG`, and `PROD` across cells or ranges.
   * Saves and loads spreadsheet data in CSV format so sheets can be reopened or inspected externally.

5. **`modules/cloud_storage.hpp` and `.cpp` — Cloud Storage**
   * Creates timestamped local backup snapshots under `data/` inside the project folder so work can be saved at specific points.
   * Supports restoring earlier snapshots and tracks sharing or sync activity logs.

6. **`modules/schedule_planner.hpp` and `.cpp` — Schedule Planner & Offline Notes**
   * Supports **Offline Mode** note taking so users can write and organize notes locally without needing external services.
   * Tracks tasks, to do items, and study schedules saved in plain text files.

7. **`modules/messaging_platform.hpp` and `.cpp` — Messaging Platform & Team Collaboration**
   * Handles direct messages and discussion threads between users.
   * Supports **Team Collaboration** through study groups and shared workspace activity for working on coursework or tasks together.

8. **`modules/online_tutor.hpp` and `.cpp` — Online Tutor**
   * Provides formula and study concept lookup along with rule based guidance.
   * Connects to a local Ollama server (`http://localhost:11434/api/chat` via `curl`, default model `llama3.2`) when available, and automatically falls back to built-in offline study and formula responses when Ollama is not running or offline.

## How the Spreadsheet Works

The `spreadsheet` module works as a terminal-based grid where you can enter values, run formulas, and export or import data as CSV:

* **Viewing and editing cells:** You navigate the grid in the terminal and update individual cells with numbers or expressions.
* **Formula evaluation (`SUM`, `AVG`, `PROD`):** You can apply supported operations over selected cells or ranges to calculate sums, averages, or products automatically.
* **CSV persistence (`data/`):** Sheets can be saved to `.csv` files inside `data/` (within the project folder) and loaded back later—and because they are standard CSV files, you can also open or inspect them in spreadsheet software or a text editor.

## Local Files Generated While Running (`data/`)

When you run the program, `data/` is created **inside the project folder** (alongside `main.cpp` and `modules/`, relative to your current working directory when running the executable) so your changes persist between sessions:

* **Where `data/` lives:** Inside your project root folder (for example, `Engitool\data\` or `shortrainbowbutterflies\data\`) whenever you run `engitool.exe` from that folder.
* **What gets stored there:** Account and profile/session data, wallet updates, offline notes, to do tasks, study schedules, CSV spreadsheets, messaging/collaboration records, and timestamped backup snapshots.
* **Why it matters:** Because everything is saved locally in plain text and CSV files, you keep your data across runs without needing an external database or cloud server.
* **Tip for Git & sharing:** If you test the app locally or build `engitool.exe`, avoid committing temporary `data/` test files or `.exe` binaries unless you intentionally want them in the repository.

## Engineering & Study Helper (Ollama + Offline Fallback)

* **Live local model:** When Ollama is running locally with a pulled model (such as `llama3.2`), the helper queries `http://localhost:11434/api/chat` via `curl`.
* **Automatic offline fallback:** If you are offline, haven't installed Ollama, or haven't started the Ollama service, the helper still works using its built-in rule-based study and engineering formula responses.
* **Offline usage:** Once models and tools are downloaded, compiling, running the app, saving `data/`, and chatting with local Ollama all work offline.

## How to Get, Open, and Run It

### Requirements

* A C++17 compiler (`g++` via MSYS2 UCRT64 / MinGW on Windows, or `g++` on Linux/macOS) and `curl` (`curl.exe` on Windows, `curl` on Linux/macOS)
* Optional: **Ollama** + a pulled model (such as `llama3.2`) for live local AI responses in the study/formula helper, **Git** (if cloning), and **VS Code** or any C++ IDE (such as Code::Blocks, CLion, or Visual Studio)
* See `REQUIREMENTS.md` for install/check commands, environment `PATH` details, and Wi-Fi vs. offline behavior.

---

### Step 1: Get the Project Folder

Choose either **Option A** (download ZIP) or **Option B** (clone with Git):

#### Option A: Download as ZIP and Extract
1. On this GitHub repository page, click the green **Code** button and select **Download ZIP**.
2. Once downloaded, right click the `.zip` file and extract the whole folder to a location you can easily find (for example, `Desktop` or `Documents`).

#### Option B: Clone via Git
1. Open PowerShell or your terminal and run:

```powershell
git clone [https://github.com/vincentd-dev/shortrainbowbutterflies.git](https://github.com/vincentd-dev/shortrainbowbutterflies.git)
cd shortrainbowbutterflies
```

---

### Step 2: Open `main.cpp` and Run the Program

After extracting the ZIP (or cloning the repo), **open the project folder** (`Open Folder`) in your IDE or terminal and go to **`main.cpp`**. Because `main.cpp` is the entry point that connects every module, compiling and running from `main.cpp` along with the module files is how you run the full program.

Choose either **Option A** (using VS Code or an IDE) or **Option B** (directly in a terminal):

#### Option A: Open Folder in VS Code or Any IDE
1. Open **VS Code** (or your preferred C++ IDE).
2. Click **File > Open Folder...** and select the extracted or cloned project folder so both `main.cpp` and the `modules/` folder are loaded.
3. Open `main.cpp`, open the integrated terminal (`Ctrl + ~` in VS Code) or configure your IDE build task to use `-std=c++17`, and compile/run using the commands in Option B.

#### Option B: Compile and Run Directly in Terminal
1. Open PowerShell or Command Prompt inside the project folder:

```powershell
cd path\to\your\project\folder
```

2. Compile `main.cpp` together with all `.cpp` files in `modules/` using C++17 (using `g++` directly if in `PATH`, or `C:\msys64\ucrt64\bin\g++.exe`):

**PowerShell:**
```powershell
$cppFiles = @("main.cpp") + (Get-ChildItem "modules\*.cpp").FullName
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -Wall -Wextra $cppFiles -o engitool.exe
.\engitool.exe
```

**CMD:**
```cmd
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -Wall -Wextra main.cpp modules\*.cpp -o engitool.exe
engitool.exe
```
