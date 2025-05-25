#include <windows.h>
#include<iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <unistd.h>
#include <limits.h>

#include "builtins.h"
#include "utils.h"
#include "config.h"
#include "shell.h"
#include "process_manager.h"
#include "history.h"
#include "io_redirection.h"
#include "autocomplete.h"

using namespace std;

static HistoryManager history;

ShellConfig shellConfig;

ProcessManager processManager;

string expandEnvironmentVariables(const string& input) {
    return shellConfig.expandVariables(input);
}

void loadConfig() {
    
    shellConfig.loadSystemEnvironment();
    
    shellConfig.loadConfigFile();
    
    shellConfig.syncWithSystemEnvironment();
}

vector<string> tokenize(const string& input) {
    vector<string> tokens;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (c == '"') {
            inQuotes = !inQuotes;  
        } else if (isspace(c) && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}
void parseRedirection(string& command, string& inputFile, string& outputFile, bool& appendMode) {
    istringstream iss(command);
    string token;
    string cleanedCommand;

    while (iss >> token) {
        if (token == "<") {
            iss >> inputFile;
        } else if (token == ">>") {
            iss >> outputFile;
            appendMode = true; 
        } else if (token == ">") {
            iss >> outputFile;
            appendMode = false; 
        } else {
            cleanedCommand += token + " ";
        }
    }

    command = trim(cleanedCommand);
}

void executeCommand(const string& commandLine){
    string line = trim(commandLine);
    if (line.empty()) return;

    string expandedLine = expandEnvironmentVariables(line);
    if (expandedLine.find('<') != string::npos || expandedLine.find('>') != string::npos) {
        executeWithRedirection(expandedLine);
        return;
    }

    vector<string> args = tokenize(expandedLine);
    if (args.empty()) return;
    
    string command = args[0];
    
    try{
        if (command == "mycd") {
            if (args.size() > 1) mycd(args[1]);
            else cerr << "Usage: mycd <path>\n";
        }
        else if(command == "mypwd"){
            mypwd();
        }
        else if(command=="myecho"){
            if (args.size() > 1) {
                string text = trim(line.substr(line.find(" ") + 1));
                cout << shellConfig.expandVariables(text) << "\n";
                } else{
                    cerr << "Usage: myecho <text>\n";
                }
        }
        else if(command=="myexit"){
            myexit();
        }
        else if (command == "myclear") {
            myclear();
        } 
        else if (command == "myls") {
            myls();
        }
        else if (command == "mycat") {
            if (args.size() > 1)
                mycat(args[1]); 
            else
                mycat();        
        }
        else if (command == "mydate") {
            mydate();
        }
        else if (command == "mymkdir") {
            if (args.size() > 1) mymkdir(args[1]);
            else cerr << "Usage: mymkdir <dirname>\n";
        }
        else if (command == "myrmdir") {
            if (args.size() > 1) myrmdir(args[1]);
            else cerr << "Usage: myrmdir <dirname>\n";
        }
        else if (command == "mycp") {
            if (args.size() > 2) mycp(args[1], args[2]);
            else cerr << "Usage: mycp <source> <destination>\n";
        }
        else if (command == "mymv") {
            if (args.size() > 2) mymv(args[1], args[2]);
            else cerr << "Usage: mymv <source> <destination>\n";
        }
        else if (command == "mytouch") {
            if (args.size() > 1) mytouch(args[1]);
            else cerr << "Usage: mytouch <filename>\n";
        }
        else if (command == "myrm") {
            if (args.size() > 1) myrm(args[1]);
            else cerr << "Usage: myrm <filename>\n";
        }
        else if (command == "mytime") {
            mytime();
        }
        else if (command == "myhelp") {
            myhelp();
        }
        else if (command == "myexport") {
            if (args.size() > 1) {
                size_t eqPos = args[1].find('=');
                if (eqPos != string::npos) {
                string var = args[1].substr(0, eqPos);
                string val = args[1].substr(eqPos + 1);
                shellConfig.setEnv(var, val);
                shellConfig.syncWithSystemEnvironment(); 
                } else {
                cerr << "Usage: myexport VAR=value\n";
                }
            } else {
                cerr << "Usage: myexport VAR=value\n";
            }
        }
        else if (command == "unset") {
            if (args.size() > 1) {
                if (!shellConfig.unsetEnv(args[1])) {
                    cerr << "Variable not found: " << args[1] << "\n";
                }
                shellConfig.syncWithSystemEnvironment();
            } else {
                cerr << "Usage: unset VAR\n";
            }
        }
        else if (command == "myenv") {
            shellConfig.printEnv();
        }
        else if (command == "setprompt") {
            if (args.size() > 1) {
                shellConfig.setEnv("PROMPT", args[1]);
                shellConfig.syncWithSystemEnvironment();
            } else {
                cerr << "Usage: setprompt <new_prompt>\n";
            }
        }
        else if (command == "killtask") {
            if (args.size() < 2) cerr << "Usage: killtask <pid|process_name>\n";
            else {
                try {
                    processManager.killProcessByPID(stoi(args[1]));
                } catch (...) {
                    processManager.killProcessByName(args[1]);
                }
            }
        } 
        else if (command == "priority") {
            if (args.size() < 3) cerr << "Usage: priority <pid|name> <level>\n";
            else {
                DWORD priority;
                if (args[2] == "low") priority = IDLE_PRIORITY_CLASS;
                else if (args[2] == "below") priority = BELOW_NORMAL_PRIORITY_CLASS;
                else if (args[2] == "normal") priority = NORMAL_PRIORITY_CLASS;
                else if (args[2] == "above") priority = ABOVE_NORMAL_PRIORITY_CLASS;
                else if (args[2] == "high") priority = HIGH_PRIORITY_CLASS;
                else if (args[2] == "realtime") priority = REALTIME_PRIORITY_CLASS;
                else {
                    cerr << "Invalid priority level.\n";
                    return;
                }
                processManager.changePriority(args[1], priority);
            }
        } 
        else if (command == "mysystemtasks") {
            processManager.listAllSystemProcesses();
        } 
        else if (command == "jobs") {
            processManager.listBackgroundProcesses();
        }
        else if (command == "fg") {
            if (args.size() > 1) processManager.bringToForeground(stoi(args[1]));
            else cerr << "Usage: fg <job_id>\n";
        } 
        else if (command == "bg") {
            if (args.size() > 1) processManager.sendToBackground(stoi(args[1]));
            else cerr << "Usage: bg <job_id>\n";
        } else {
            string fullCmd = "cmd.exe /C \"" + line + "\"";
            char cmd[4096];
            strcpy(cmd, fullCmd.c_str());

            STARTUPINFOA si = {};
            PROCESS_INFORMATION pi = {};
            si.cb = sizeof(si);

            if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                cerr << "Failed to execute external command.\n";
                return;
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } 
    } catch(const exception& e){
        cerr << "Exception: " << e.what() << "\n";
    }
}

void clearAndRedrawLine(const string& prompt, const string& input, size_t cursorPos) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD written;

    GetConsoleScreenBufferInfo(hConsole, &csbi);

    COORD lineStart = { 0, csbi.dwCursorPosition.Y };

    FillConsoleOutputCharacter(hConsole, ' ', csbi.dwSize.X, lineStart, &written);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, csbi.dwSize.X, lineStart, &written);

    SetConsoleCursorPosition(hConsole, lineStart);

    cout << prompt << input << flush;

    SetConsoleCursorPosition(hConsole, { (SHORT)(prompt.length() + cursorPos), csbi.dwCursorPosition.Y });
}


void shellLoop() {
    string input;
    cout << "Welcome to MyShell! Type 'myexit' to quit.\n";

    loadConfig();

    while (true) {
        char cwd[MAX_PATH];
        string prompt;

        if (GetCurrentDirectoryA(MAX_PATH, cwd)) {
            string configPrompt = shellConfig.getEnv("PROMPT");
            prompt = configPrompt.empty() ? string(cwd) + "> " : configPrompt;
        } else {
            prompt = shellConfig.getEnv("PROMPT").empty() ? "myshell> " : shellConfig.getEnv("PROMPT");
        }

        cout << prompt << flush;

        input.clear();
        size_t cursorPos = 0;

        while (true) {
            char ch = _getch();

            if (ch == 9) { 
                size_t lastSpace = input.find_last_of(" ");
                string prefix = (lastSpace == string::npos) ? input : input.substr(lastSpace + 1);

                vector<string> completions = getCompletions(prefix);
                if (!completions.empty()) {
                    string completion = completions[0];
                    input = (lastSpace == string::npos ? "" : input.substr(0, lastSpace + 1)) + completion;
                    cursorPos = input.length();
                    clearAndRedrawLine(prompt, input, cursorPos);
                }
            } else if (ch == 13) { 
                cout << endl;
                break;
            } else if (ch == 8) { 
                if (cursorPos > 0) {
                    input.erase(cursorPos - 1, 1);
                    cursorPos--;
                    clearAndRedrawLine(prompt, input, cursorPos);
                }
            } else if (ch == -32 || ch == 224) { 
                char arrow = _getch();
                if (arrow == 72) { 
                    string prevCmd = history.getPreviousCommand();
                    if (!prevCmd.empty()) {
                        input = prevCmd;
                        cursorPos = input.length();
                        clearAndRedrawLine(prompt, input, cursorPos);
                    }
                } else if (arrow == 80) { 
                    string nextCmd = history.getNextCommand();
                    input = nextCmd;
                    cursorPos = input.length();
                    clearAndRedrawLine(prompt, input, cursorPos);
                } else if (arrow == 75) { 
                    if (cursorPos > 0) {
                        cursorPos--;
                        clearAndRedrawLine(prompt, input, cursorPos);
                    }
                } else if (arrow == 77) { 
                    if (cursorPos < input.length()) {
                        cursorPos++;
                        clearAndRedrawLine(prompt, input, cursorPos);
                    }
                }
            } else {
                input.insert(cursorPos, 1, ch);
                cursorPos++;
                clearAndRedrawLine(prompt, input, cursorPos);
            }
        }

        if (!input.empty()) {
            history.addCommand(input);
        }

        executeCommand(input);
    }

    history.saveHistoryToFile();
}