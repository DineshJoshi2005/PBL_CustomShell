#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include "utils.h"
#include "shell.h"
#include "io_redirection.h"

using namespace std;

void executeWithRedirection(const string& commandLine) {
    string commandPart = commandLine;
    string inputFile, outputFile;
    bool appendMode = false; 

    parseRedirection(commandPart, inputFile, outputFile, appendMode);

    int originalStdin = _dup(_fileno(stdin));
    int originalStdout = _dup(_fileno(stdout));

    FILE* inFile = nullptr;
    FILE* outFile = nullptr;

    if (!inputFile.empty()) {
        inFile = freopen(inputFile.c_str(), "r", stdin);
        if (!inFile) {
            cerr << "Error: Cannot open input file '" << inputFile << "'\n";
            return;
        }
    }

    if (!outputFile.empty()) {
        const char* mode = appendMode ? "a" : "w";
        outFile = freopen(outputFile.c_str(), mode, stdout);
        if (!outFile) {
            cerr << "Error: Cannot open output file '" << outputFile << "'\n";
            if (inFile) fclose(inFile);
            return;
        }
    }

    executeCommand(commandPart);

    fflush(stdout);
    fflush(stdin);

    if (!inputFile.empty()) {
        freopen("CON", "r", stdin);
        _dup2(originalStdin, _fileno(stdin));
    }

    if (!outputFile.empty()) {
        freopen("CON", "w", stdout);
        _dup2(originalStdout, _fileno(stdout));
    }

    _close(originalStdin);
    _close(originalStdout);
}



