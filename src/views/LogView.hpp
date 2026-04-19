#ifndef LOG_VIEW_HPP
#define LOG_VIEW_HPP

#include <cstddef>
#include <string>

class LogEntry;

class LogView {
private:
    int maxPreviewWidth;

public:
    LogView(int maxPreviewWidth = 120);

    void showLogs(const LogEntry* logs, std::size_t count) const;
    void showLastLogs(const LogEntry* logs, std::size_t count, int n) const;
    std::string formatEntry(const LogEntry& entry) const;
};

#endif
