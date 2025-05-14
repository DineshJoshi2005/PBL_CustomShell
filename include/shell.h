#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>

void shellLoop();
void executeCommand(const std::string& commandLine);
#endif // SHELL_H