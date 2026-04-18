#include "views/LogView.hpp"

#include <iostream>
#include <sstream>

LogView::LogView(int maxPreviewWidth) : maxPreviewWidth(maxPreviewWidth) {}

std::string LogView::formatEntry(const LogEntry* entry) const {
    if (entry == nullptr) {
        return "[LOG INVALID]";
    }

    std::stringstream ss;
    ss << "[Turn " << entry->getTurn() << "] "
       << entry->getUsername() << " | "
       << entry->getActionType() << " | "
       << entry->getDetail();

    std::string result = ss.str();

    if (static_cast<int>(result.length()) > maxPreviewWidth) {
        result = result.substr(0, maxPreviewWidth - 3) + "...";
    }

    return result;
}

void LogView::showLogs(const std::vector<LogEntry*>& logs) const {
    std::cout << "=== Log Transaksi Penuh ===\n";

    if (logs.empty()) {
        std::cout << "Belum ada log.\n";
        return;
    }

    for (const LogEntry* entry : logs) {
        std::cout << formatEntry(entry) << '\n';
    }
}

void LogView::showLastLogs(const std::vector<LogEntry*>& logs, int n) const {
    if (n <= 0) {
        std::cout << "Jumlah log harus lebih dari 0.\n";
        return;
    }

    std::cout << "=== Log Transaksi (" << n << " Terakhir) ===\n";

    if (logs.empty()) {
        std::cout << "Belum ada log.\n";
        return;
    }

    int start = 0;
    if (static_cast<int>(logs.size()) > n) {
        start = static_cast<int>(logs.size()) - n;
    }

    for (int i = start; i < static_cast<int>(logs.size()); ++i) {
        std::cout << formatEntry(logs[i]) << '\n';
    }
}