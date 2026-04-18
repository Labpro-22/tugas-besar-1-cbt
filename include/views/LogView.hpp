#ifndef LOG_VIEW_HPP
#define LOG_VIEW_HPP

#include <string>
#include <vector>

#include "models/GameManager/LogEntry.hpp"

class LogView {
private:
    int maxPreviewWidth;

public:
    LogView(int maxPreviewWidth = 120);

    void showLogs(const std::vector<LogEntry*>& logs) const;
    void showLastLogs(const std::vector<LogEntry*>& logs, int n) const;
    std::string formatEntry(const LogEntry* entry) const;
};

#endif