#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

DWORD GetProcessIdByName(const wchar_t* processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry)) {
        do {
            if (wcscmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

uintptr_t GetMainModuleBaseAddress(DWORD processId) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snapshot, &moduleEntry)) {
        CloseHandle(snapshot);
        return reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
    }

    CloseHandle(snapshot);
    return 0;
}

void CenterConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    RECT consoleRect;
    GetWindowRect(consoleWindow, &consoleRect);

    int consoleWidth = consoleRect.right - consoleRect.left;
    int consoleHeight = consoleRect.bottom - consoleRect.top;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int consoleX = (screenWidth - consoleWidth) / 2;
    int consoleY = (screenHeight - consoleHeight) / 2;

    SetWindowPos(consoleWindow, 0, consoleX, consoleY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

int main() {
    // Set the console title
    SetConsoleTitle(L"Address Finder");

    CenterConsoleWindow();

    wchar_t targetProcessName[MAX_PATH];

    std::wcout << L"Enter the target process name: ";
    std::wcin.getline(targetProcessName, MAX_PATH);

    // Clear the console content
    system("cls");

    DWORD processId = GetProcessIdByName(targetProcessName);

    if (processId != 0) {
        uintptr_t mainModuleBaseAddress = GetMainModuleBaseAddress(processId);

        if (mainModuleBaseAddress != 0) {
            std::wcout << L"Base address of the main module in the process " << targetProcessName << L": 0x" << std::hex << mainModuleBaseAddress << std::endl;
        }
        else {
            std::wcerr << L"Could not obtain the base address of the main module." << std::endl;
        }
    }
    else {
        std::wcerr << L"The process was not found." << std::endl;
    }

    // Pause to wait for user input before closing the console
    std::wcout << L"Press Enter to close...";
    std::wcin.get();

    return 0;
}
