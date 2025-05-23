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

using namespace std;

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

void executeCommand(const string& commandLine){
    string line = trim(commandLine);
    if (line.empty()) return;

    string expandedLine = expandEnvironmentVariables(line);


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
                cout << text << "\n";
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
            if (line.find('<') != string::npos)
                mycat(trim(line.substr(line.find('<') + 1)));
            else if (args.size() > 1)
                mycat(args[1]);
            else
                cerr << "Usage: mycat <filename>\n";
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
        }  
    } catch(const exception& e){
        cerr << "Exception: " << e.what() << "\n";
    }
}

void shellLoop(){
    string input;
    cout<< "Welcome to MyShell! Type 'myexit' to quit.\n";
        while(true){
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
                while (true) {
                    char ch = _getch();

                    if (ch == 13) { 
                        cout << endl;
                        break;
                    } else if (ch == 8) { 
                        if (!input.empty()) {
                            input.pop_back();
                            cout << "\b \b";
                        }
                    } else {
                        input += ch;
                        cout << ch;
                    }
                }
            executeCommand(input);
        }
}