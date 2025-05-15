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

using namespace std;

ShellConfig shellConfig;

string expandEnvironmentVariables(const std::string& input) {
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

    std::string expandedLine = expandEnvironmentVariables(line);


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
            if (line.find('<') != std::string::npos)
                mycat(trim(line.substr(line.find('<') + 1)));
            else if (args.size() > 1)
                mycat(args[1]);
            else
                std::cerr << "Usage: mycat <filename>\n";
        }
        else if (command == "mydate") {
            mydate();
        }
        else if (command == "mymkdir") {
            if (args.size() > 1) mymkdir(args[1]);
            else std::cerr << "Usage: mymkdir <dirname>\n";
        }
        else if (command == "myrmdir") {
            if (args.size() > 1) myrmdir(args[1]);
            else std::cerr << "Usage: myrmdir <dirname>\n";
        }
        else if (command == "mycp") {
            if (args.size() > 2) mycp(args[1], args[2]);
            else std::cerr << "Usage: mycp <source> <destination>\n";
        }
        else if (command == "mymv") {
            if (args.size() > 2) mymv(args[1], args[2]);
            else std::cerr << "Usage: mymv <source> <destination>\n";
        }
        else if (command == "mytouch") {
            if (args.size() > 1) mytouch(args[1]);
            else std::cerr << "Usage: mytouch <filename>\n";
        }
        else if (command == "myrm") {
            if (args.size() > 1) myrm(args[1]);
            else std::cerr << "Usage: myrm <filename>\n";
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
                if (eqPos != std::string::npos) {
                std::string var = args[1].substr(0, eqPos);
                std::string val = args[1].substr(eqPos + 1);
                shellConfig.setEnv(var, val);
                shellConfig.syncWithSystemEnvironment(); 
                } else {
                std::cerr << "Usage: myexport VAR=value\n";
                }
            } else {
                std::cerr << "Usage: myexport VAR=value\n";
            }
        }
        else if (command == "unset") {
            if (args.size() > 1) {
                if (!shellConfig.unsetEnv(args[1])) {
                    std::cerr << "Variable not found: " << args[1] << "\n";
                }
                shellConfig.syncWithSystemEnvironment();
            } else {
                std::cerr << "Usage: unset VAR\n";
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
                std::cerr << "Usage: setprompt <new_prompt>\n";
            }
        }  
    } catch(const exception& e){
        cerr << "Exception: " << e.what() << "\n";
    }
}

void shellLoop(){
    string input;
    cout<< "Welcome to MyShell! Type 'myexit' to quit.\n";
        while(true){
            input.clear();
                while (true) {
                    char ch = _getch();

                    if (ch == 13) { 
                        std::cout << std::endl;
                        break;
                    } else if (ch == 8) { 
                        if (!input.empty()) {
                            input.pop_back();
                            std::cout << "\b \b";
                        }
                    } else {
                        input += ch;
                        std::cout << ch;
                    }
                }
            executeCommand(input);
        }
}