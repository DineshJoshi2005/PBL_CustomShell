#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

class HistoryManager {
private:
    std::vector<std::string> history;
    int currentIndex;
    std::string historyFile;

public:
    HistoryManager(const std::string& file = ".myshell_history");

    void addCommand(const std::string& command);
    std::string getPreviousCommand();
    std::string getNextCommand();
    void loadHistoryFromFile();
    void saveHistoryToFile();
};

#endif
