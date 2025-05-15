#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class ShellConfig {
public:
    ShellConfig();
    ~ShellConfig() = default;

    void setEnv(const string& var, const string& value);
    string getEnv(const string& var) const;
    void printEnv() const;
    bool unsetEnv(const string& var);
    
    void loadConfigFile(const string& filePath = ".myshellrc");
    void saveConfigFile(const string& filePath = ".myshellrc") const;
    
    string expandVariables(const string& input) const;

    void loadSystemEnvironment();
    void syncWithSystemEnvironment() const;

private:
    unordered_map<string, string> env_vars;
    
    void parseConfigLine(const string& line);
    vector<string> tokenizeConfigLine(const string& line) const;
    bool isValidVarName(const string& name) const;
};

#endif 