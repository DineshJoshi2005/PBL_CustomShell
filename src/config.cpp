#include "config.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
using namespace std;

ShellConfig::ShellConfig() {
    loadSystemEnvironment();
}

void ShellConfig::setEnv(const string& var, const string& value) {
    if (isValidVarName(var)) {
        env_vars[var] = value;
    }
}

string ShellConfig::getEnv(const string& var) const {
    auto it = env_vars.find(var);
    if (it != env_vars.end()) {
        return it->second;
    }
    return "";
}

bool ShellConfig::unsetEnv(const string& var) {
    return env_vars.erase(var) > 0;
}

void ShellConfig::printEnv() const {
    for (const auto& pair : env_vars) {
        cout << pair.first << "=" << pair.second << endl;
    }
}



void ShellConfig::loadConfigFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line[0] == '#') continue;

        size_t equalPos = line.find('=');
        if (equalPos == string::npos) continue;

        string key = line.substr(0, equalPos);
        string value = line.substr(equalPos + 1);

        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);

        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        if (!value.empty() && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2); 
        }

        env_vars[key] = value;
    }

    file.close();
}

void ShellConfig::saveConfigFile(const string& filePath) const {
    ofstream configFile(filePath);
    if (!configFile.is_open()) {
        cerr << "Error: Could not save configuration to " << filePath << endl;
        return;
    }

    for (const auto& pair : env_vars) {
        configFile << pair.first << "=" << pair.second << "\n";
    }
}

void ShellConfig::loadSystemEnvironment() {
    extern char **environ;
    for (char **env = environ; *env != nullptr; env++) {
        string envStr(*env);
        size_t equalsPos = envStr.find('=');
        if (equalsPos != string::npos) {
            string var = envStr.substr(0, equalsPos);
            string value = envStr.substr(equalsPos + 1);
            env_vars[var] = value;
        }
    }
}

void ShellConfig::syncWithSystemEnvironment() const {
    for (const auto& pair : env_vars) {
        #ifdef _WIN32
        _putenv_s(pair.first.c_str(), pair.second.c_str());
        #else
        setenv(pair.first.c_str(), pair.second.c_str(), 1);
        #endif
    }
}

string ShellConfig::expandVariables(const string& input) const {
    string result;
    size_t start = 0;
    size_t dollarPos = input.find('$', start);

    while (dollarPos != string::npos) {
        result += input.substr(start, dollarPos - start);

        if (dollarPos + 1 < input.size() && input[dollarPos + 1] == '{') {
            size_t endBrace = input.find('}', dollarPos + 2);
            if (endBrace != string::npos) {
                string varName = input.substr(dollarPos + 2, endBrace - dollarPos - 2);
                result += getEnv(varName);
                start = endBrace + 1;
            } else {
                result += input.substr(dollarPos, 2);
                start = dollarPos + 2;
            }
        } else {
            size_t varEnd = dollarPos + 1;
            while (varEnd < input.size() && (isalnum(input[varEnd]) || input[varEnd] == '_')) {
                varEnd++;
            }
            string varName = input.substr(dollarPos + 1, varEnd - dollarPos - 1);
            result += getEnv(varName);
            start = varEnd;
        }

        dollarPos = input.find('$', start);
    }

    result += input.substr(start);
    return result;
}

void ShellConfig::parseConfigLine(const string& line) {
    vector<string> tokens = tokenizeConfigLine(line);
    if (tokens.empty()) return;

    if (tokens[0] == "export" && tokens.size() >= 2) {
        size_t equalsPos = tokens[1].find('=');
        if (equalsPos != string::npos) {
            string var = tokens[1].substr(0, equalsPos);
            string value = tokens[1].substr(equalsPos + 1);
            setEnv(var, value);
        }
    } else if (tokens.size() >= 1) {
        size_t equalsPos = tokens[0].find('=');
        if (equalsPos != string::npos) {
            string var = tokens[0].substr(0, equalsPos);
            string value = tokens[0].substr(equalsPos + 1);
            setEnv(var, value);
        }
    }
}

vector<string> ShellConfig::tokenizeConfigLine(const string& line) const {
    vector<string> tokens;
    istringstream iss(line);
    string token;
    
    while (iss >> token) {
        if (token.front() == '"' || token.front() == '\'') {
            char quote = token.front();
            string fullToken = token.substr(1);
            
            if (token.back() != quote) {
                string nextToken;
                while (iss >> nextToken) {
                    fullToken += " " + nextToken;
                    if (nextToken.back() == quote) {
                        fullToken = fullToken.substr(0, fullToken.size() - 1);
                        break;
                    }
                }
            } else {
                fullToken = fullToken.substr(0, fullToken.size() - 1);
            }
            
            tokens.push_back(fullToken);
        } else {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

bool ShellConfig::isValidVarName(const string& name) const {
    if (name.empty()) return false;
    
    if (!isalpha(name[0]) && name[0] != '_') {
        return false;
    }
    
    for (char c : name) {
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    
    return true;
}