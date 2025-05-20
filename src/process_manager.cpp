#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <tlhelp32.h>
#include <iostream>
#include "process_manager.h"


void ProcessManager::addBackgroundProcess(PROCESS_INFORMATION pi) {
    backgroundProcesses.push_back(pi);
}

void ProcessManager::listBackgroundProcesses() {
    if (backgroundProcesses.empty()) {
        std::cout << "No background processes.\n";
        return;
    }

    std::cout << "Background processes:\n";
    for (size_t i = 0; i < backgroundProcesses.size(); ++i) {
        std::cout << "Job ID: " << i + 1 << "\n";
        std::cout << "PID: " << backgroundProcesses[i].dwProcessId << "\n";
    }
}

void ProcessManager::bringToForeground(int jobId) {
    if (jobId <= 0 || jobId > backgroundProcesses.size()) {
        std::cerr << "Invalid job ID.\n";
        return;
    }

    PROCESS_INFORMATION pi = backgroundProcesses[jobId - 1];
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Optionally, remove from background list after completion
    backgroundProcesses.erase(backgroundProcesses.begin() + jobId - 1);
}

void ProcessManager::sendToBackground(int jobId) {
    if (jobId <= 0 || jobId > backgroundProcesses.size()) {
        std::cerr << "Invalid job ID.\n";
        return;
    }

    // The background process will continue running.
    std::cout << "Process " << jobId << " sent to background.\n";
}

void ProcessManager::listAllSystemProcesses() { 
    DWORD processes[1024], cbNeeded, processCount;

    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        std::cerr << "Failed to enumerate processes.\n";
        return;
    }

    processCount = cbNeeded / sizeof(DWORD);

    std::cout << "PID\t\tProcess Name\n";
    std::cout << "----------------------------------\n";

    for (unsigned int i = 0; i < processCount; ++i) {
        if (processes[i] == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
        if (hProcess) {
            HMODULE hMod;
            DWORD cbNeededMod;

            char processName[MAX_PATH] = "<unknown>";

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                GetModuleBaseNameA(hProcess, hMod, processName, sizeof(processName) / sizeof(char));
            }

            std::cout << processes[i] << "\t\t" << processName << "\n";

            CloseHandle(hProcess);
        }
    }
}
void ProcessManager::killProcessByPID(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process with PID " << pid << ". Error code: " << GetLastError() << "\n";
        return;
    }

    if (!TerminateProcess(hProcess, 1)) {
        std::cerr << "Failed to terminate process with PID " << pid << ". Error code: " << GetLastError() << "\n";
    } else {
        std::cout << "Successfully terminated process with PID " << pid << ".\n";
    }

    CloseHandle(hProcess);
}

void ProcessManager::killProcessByName(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Unable to create process snapshot.\n";
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "Error: Unable to retrieve process information.\n";
        CloseHandle(hSnapshot);
        return;
    }

    int killCount = 0;

    do {
        if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                if (TerminateProcess(hProcess, 0)) {
                    std::cout << "Killed process: " << pe32.szExeFile << " [PID: " << pe32.th32ProcessID << "]\n";
                    ++killCount;
                } else {
                    std::cerr << "Failed to kill process with PID: " << pe32.th32ProcessID << "\n";
                }
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    if (killCount == 0) {
        std::cout << "No process named '" << processName << "' was found.\n";
    }

    CloseHandle(hSnapshot);
}
void ProcessManager::changePriority(const std::string& target, DWORD priority) {
    try {
        DWORD pid = std::stoi(target);
        changePriorityByPID(pid, priority);
    } catch (...) {
        changePriorityByName(target, priority);
    }
}
void ProcessManager::changePriorityByPID(DWORD pid, DWORD priority) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (hProcess) {
        if (SetPriorityClass(hProcess, priority)) {
            std::cout << "Priority changed for PID " << pid << "\n";
        } else {
            std::cerr << "Failed to change priority for PID " << pid << "\n";
        }
        CloseHandle(hProcess);
    } else {
        std::cerr << "Unable to open process PID " << pid << "\n";
    }
}
void ProcessManager::changePriorityByName(const std::string& processName, DWORD priority) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    bool found = false;

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName.c_str()) == 0) {
                found = true;
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    if (SetPriorityClass(hProcess, priority)) {
                        std::cout << "Priority changed for " << processName
                                  << " [PID: " << pe.th32ProcessID << "]\n";
                    } else {
                        std::cerr << "Failed to change priority for PID " << pe.th32ProcessID << "\n";
                    }
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    if (!found) {
        std::cerr << "No process found with name " << processName << "\n";
    }

    CloseHandle(hSnapshot);
}