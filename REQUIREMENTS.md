# Requirements

## What to Install & Wi-Fi Info
- **Required:** C++17 compiler (`g++`, such as MSYS2 UCRT64 `C:\msys64\ucrt64\bin\g++.exe` on Windows or `g++` / `clang++` on Linux/macOS) and `curl` (`curl.exe` on Windows 10/11, `curl` on Linux/macOS).
- **Optional (Live Local LLM):** Ollama + a pulled model (such as `llama3.2`) for the Engineering & Study Helper. If Ollama is not running or no model is pulled, the app automatically falls back to built-in offline study and formula responses.
- **Wi-Fi / Offline:** Wi-Fi is only needed for initial downloads (repository/ZIP, compiler/tools, Ollama installer, and pulling models). Compiling, running the app, saving local `data/` files, and chatting with an already-downloaded local Ollama model all work offline.

## Install & Check Commands

### OPTION A: PowerShell (Recommended)
```powershell
C:\msys64\ucrt64\bin\g++.exe --version
curl.exe --version
winget install Ollama.Ollama
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
ollama pull llama3.2
```

### OPTION B: CMD
```cmd
C:\msys64\ucrt64\bin\g++.exe --version
curl.exe --version
winget install Ollama.Ollama
"%LOCALAPPDATA%\Programs\Ollama\ollama.exe" pull llama3.2
```

### OPTION 3: Linux / macOS
```bash
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -Wall -Wextra main.cpp modules\*.cpp -o engitool.exe
engitool.exe
```

See `README.md` for building and running the application.
