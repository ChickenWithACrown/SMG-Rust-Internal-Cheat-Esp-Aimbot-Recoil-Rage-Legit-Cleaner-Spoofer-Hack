#include "tools.h"
#include <atomic>
#include <cstdlib>
#include <cstring>

// Function to get the process ID (PID) by process name
uint32_t get_process_pid(const char* process_name) {
    PROCESSENTRY32 process_entry{};
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    uint32_t pid = 0;

    // Create a snapshot of all processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    // Iterate through all processes
    if (Process32First(snapshot, &process_entry)) {
        do {
            if (strcmp(process_name, process_entry.szExeFile) == 0) {
                pid = process_entry.th32ProcessID;
                CloseHandle(snapshot);
                return pid;
            }
        } while (Process32Next(snapshot, &process_entry));
    }

    CloseHandle(snapshot);
    return 0;
}

// Function to generate a random alphanumeric string
char* randomStrGen(int length) {
    static std::atomic<int> id{ 0 };  // Atomic counter for thread-safe unique ID
    int inc = id.fetch_add(1, std::memory_order_relaxed) + 1;

    // Seed the random number generator with a unique value
    srand(static_cast<unsigned>(GetTickCount() ^ (inc * inc + inc)));

    static const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    std::string result(length, ' ');  // Pre-allocate string of the desired length

    // Generate a random string
    for (int i = 0; i < length; ++i) {
        result[i] = charset[rand() % charset.length()];
    }

    // Allocate memory and copy string
    char* Cresult = (char*)malloc(result.size() + 1);
    if (Cresult) {
        strcpy_s(Cresult, result.size() + 1, result.c_str());
    }
    return Cresult;
}

// Function to get the desktop resolution (horizontal, vertical)
void GetDesktopResolution(int& horizontal, int& vertical) {
    RECT desktop;

    // Get the handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    
    // Get the screen size and store it in 'desktop'
    if (GetWindowRect(hDesktop, &desktop)) {
        horizontal = desktop.right;
        vertical = desktop.bottom;
    } else {
        horizontal = vertical = 0;  // In case of error, set resolution to 0
    }
}
