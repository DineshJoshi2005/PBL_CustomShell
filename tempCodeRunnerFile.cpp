#include "autocomplete.h"
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

namespace fs = filesystem;
vector<string> getMatchingFilenames(const string& prefix) {
    vector<string> matches;

    string path = ".";
    string filePrefix = prefix;

    auto lastSlash = prefix.find_last_of("/\\");
    if (lastSlash != string::npos) {
        path = prefix.substr(0, lastSlash + 1);
        filePrefix = prefix.substr(lastSlash + 1);
    } else {
        path = "";  
    }

    try {
        fs::path searchPath = (path.empty() ? "." : path);

        for (const auto& entry : fs::directory_iterator(searchPath)) {
            string filename = entry.path().filename().string();

            if (filename.rfind(filePrefix, 0) == 0) {
                string result = path + filename;

                
                if (result.find(' ') != string::npos ||
                    result.find('(') != string::npos ||
                    result.find(')') != string::npos ||
                    result.find('&') != string::npos ||
                    result.find('\'') != string::npos) {
                    result = "\"" + result + "\"";
                }

                matches.push_back(result);
            }
        }
    } catch (const fs::filesystem_error& e) {
        
    }

    return matches;
}


vector<string> getCompletions(const string& prefix) {
    return getMatchingFilenames(prefix);
}