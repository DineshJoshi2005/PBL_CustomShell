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

void mycd(const string& path) {
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

void myclear() {
    system("cls");
}


void myls() {
    for (const auto& entry : filesystem::directory_iterator(filesystem::current_path())) {
        cout << entry.path().filename().string() << "\n";
    }
}

void mycat(const string& filename) {
    istream* in = &cin;
    ifstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file '" << filename << "'\n";
            return;
        }
        in = &file;
    }

    string line;
    while (getline(*in, line)) {
        cout << line << "\n";
    }
}



void mydate() {
    time_t t = time(nullptr);
    cout << asctime(localtime(&t));
}


void mymkdir(const string& dirname) {
    wstring wdirname = to_wstring(dirname);
    if (!CreateDirectoryW(wdirname.c_str(), NULL)) {
        cerr << "Error: Unable to create directory " << dirname << "\n";
    }
}


void mytime() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    cout << "Current time: " 
    << put_time(now, "%H:%M:%S") << "\n";
}


void myhelp() {
    cout << "List of supported commands:\n";
    cout << "  mycd <path>      - Change directory\n";
    cout << "  mypwd            - Print current directory\n";
    cout << "  myecho <text>    - Print text to the console\n";
    cout << "  myexit           - Exit the shell\n";
    cout << "  myclear          - Clear the screen\n";
    cout << "  myls             - List files in the current directory\n";
    cout << "  mycat <file>     - Display contents of a file\n";
    cout << "  mydate           - Show current date and time\n";
    cout << "  mymkdir <dir>    - Create a new directory\n";
    cout << "  myrmdir <dir>    - Remove an empty directory\n";
    cout << "  mymv <src> <dst> - Rename or move a file\n";
    cout << "  mycp <src> <dst> - Copy a file\n";
    cout << "  mytime           - Show the current system time\n";
    cout << "  mytouch <file>   - Create a new file (like touch in Linux)\n";
    cout << "  myhelp           - Show this help message\n";
    cout << "  myrm <file>      - Remove a file\n";
}


void mymv(const string& src, const string& dest) {
    wstring wsrc = to_wstring(src);
    wstring wdest = to_wstring(dest);
    if (!MoveFileW(wsrc.c_str(), wdest.c_str())) {
        cerr << "Error: Unable to move/rename " << src << "\n";
    }
}


void mycp(const string& src, const string& dest) {
    wstring wsrc = to_wstring(src);
    wstring wdest = to_wstring(dest);
    if (!CopyFileW(wsrc.c_str(), wdest.c_str(), FALSE)) {
        cerr << "Error: Unable to copy file " << src << "\n";
    }
}


void myrmdir(const string& dirname) {
    wstring wdirname = to_wstring(dirname);
    if (!RemoveDirectoryW(wdirname.c_str())) {
        cerr << "Error: Unable to remove directory " << dirname << "\n";
    }
}


void mytouch(const string& filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error: Could not create file " << filename << "\n";
    }
    
}


void myrm(const string& filename) {
    try {
        if (filesystem::remove(filename)) {
            cout << "File removed: " << filename << "\n";
        } else {
            cerr << "Error: File not found or cannot be removed: " << filename << "\n";
        }
    } catch (const filesystem::filesystem_error& e) {
        cerr << "Filesystem error: " << e.what() << "\n";
    }
}
