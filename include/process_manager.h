#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <windows.h>
#include <vector>
#include <iostream>
using namespace std;

class ProcessManager {
public:
    void addBackgroundProcess(PROCESS_INFORMATION pi);
    void listBackgroundProcesses();
    void bringToForeground(int jobId);
    void sendToBackground(int jobId);
    void listAllSystemProcesses();
    void killProcessByPID(DWORD pid);
    void killProcessByName(const string& processName);
    void changePriority(const string& nameOrPid, DWORD priority);
    void changePriorityByPID(DWORD pid, DWORD priority);
    void changePriorityByName(const string& name, DWORD priority);

private:
    vector<PROCESS_INFORMATION> backgroundProcesses;
};
#endif 