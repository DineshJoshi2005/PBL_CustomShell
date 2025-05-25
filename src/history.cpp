#include "history.h"
#include <fstream>
#include <iostream>

HistoryManager::HistoryManager(const std::string& file)
    : currentIndex(-1), historyFile(file) {
    loadHistoryFromFile();
}

void HistoryManager::addCommand(const std::string& command) {
    if (!command.empty()) {
        history.push_back(command);
        currentIndex = history.size();  // Reset index after new command
    }
}

std::string HistoryManager::getPreviousCommand() {
    if (currentIndex > 0) {
        currentIndex--;
        return history[currentIndex];
    }
    return "";
}

std::string HistoryManager::getNextCommand() {
    if (currentIndex < static_cast<int>(history.size()) - 1) {
        currentIndex++;
        return history[currentIndex];
    }
    return "";
}

void HistoryManager::loadHistoryFromFile() {
    std::ifstream infile(historyFile);
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty()) {
            history.push_back(line);
        }
    }
    currentIndex = history.size();
}

void HistoryManager::saveHistoryToFile() {
    std::ofstream outfile(historyFile);
    for (const auto& cmd : history) {
        outfile << cmd << '\n';
    }
}
