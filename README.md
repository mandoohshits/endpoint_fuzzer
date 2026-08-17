Endpoint & Directory Fuzzer
A high-performance, multithreaded web endpoint and directory fuzzing tool written in C++ using wxWidgets. Designed for web application security assessments, penetration testing, and reconnaissance, this tool allows you to scan web servers for hidden files, administrative paths, and exposed endpoints.

Key Features
Native Desktop GUI: Built with wxWidgets for a clean, responsive native interface on Windows.

Multithreaded Scanning: Fast, non-blocking requests using background threads to ensure the UI stays smooth during large wordlist scans.

Stand-Alone Portability: Statically compiled binary (fuzzer.exe) with zero external DLL dependencies required on the target machine.

Custom Wordlists & Extensions: Supports custom wordlists (.txt) and file extension appending (e.g., .php, .html, .json).

Real-time Results: Displays HTTP status codes, full target URLs, and response metrics as endpoints are discovered.

Getting Started
Prerequisites (For Building from Source)
To compile the C++ source code yourself, you need:

MSYS2 (UCRT64 environment)

GCC / G++ Compiler (mingw-w64-ucrt-x86_64-gcc)

wxWidgets 3.2+ (mingw-w64-ucrt-x86_64-wxwidgets3.2-msw)

Compilation (Static Build)
Open the MSYS2 UCRT64 terminal.

Navigate to the project directory:

Bash
cd /path/to/endpoint_fuzzer
Compile the application with static linking:

Bash
g++ fuzzer.c -o fuzzer.exe -static -static-libgcc -static-libstdc++ `wx-config --cxxflags --libs --static=yes`
Usage
Launch fuzzer.exe by double-clicking or executing ./fuzzer.exe in the terminal.

Enter the Base URL (e.g., [http://example.com](http://example.com) or [http://192.168.1.100](http://192.168.1.100)).

Select your wordlist file (.txt).

(Optional) Specify file extensions to append (comma-separated, e.g., php, html, txt).

Click Start Fuzzing to launch the scan.

Project Structure
Plaintext
├── fuzzer.c          # C++ source code (wxWidgets GUI & scanner logic)
├── fuzzer.exe        # Statically compiled executable
├── README.md         # Documentation
└── .gitignore        # Build artifacts and binary exclusions
