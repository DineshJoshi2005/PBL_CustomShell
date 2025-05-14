#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include "builtins.h"
using namespace std;

wstring to_wstring(const string& str) {
    return wstring(str.begin(), str.end());
}

void mycd(const std::string& path) {
    wstring wpath = to_wstring(path);
    if (!SetCurrentDirectoryW(wpath.c_str())) {
        cerr << "Error: Unable to change directory to " << path << "\n";
    }
}

void mypwd() {
    wchar_t cwd[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, cwd)) {
        wcout << cwd << "\n";
    } else {
        cerr << "Error: Unable to get current directory\n";
    }
}

void myecho(const string& text) {
    cout << text << "\n";
}

void myexit() {
    cout << "Exiting the shell...\n";
    exit(0);
}
