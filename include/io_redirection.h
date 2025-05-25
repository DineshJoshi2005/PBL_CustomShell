#ifndef IO_REDIRECTION_H
#define IO_REDIRECTION_H

#include <string>
using namespace std;
void executeWithRedirection(const string& line);
void parseRedirection(string& command, string& inputFile, string& outputFile);

#endif
