#include "views/LogView.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "exception/NimonspoliExceptions.hpp"
#include "models/GameManager/LogEntry.hpp"

LogView::LogView(int maxPreviewWidth) : maxPreviewWidth(maxPreviewWidth) {}

std::string LogView::formatEntry(const LogEntry &entry) const {
  std::stringstream ss;
  ss << "[Turn " << entry.turn << "] " << entry.username << " | "
     << entry.actionType << " | " << entry.detail;

  std::string result = ss.str();

  if (maxPreviewWidth > 3 &&
      static_cast<int>(result.length()) > maxPreviewWidth) {
    result = result.substr(0, maxPreviewWidth - 3) + "...";
  }

  return result;
}

void LogView::showLogs(const LogEntry *logs, std::size_t count) const {
  std::cout << "=== Log Transaksi Penuh ===\n";

  if (count == 0) {
    std::cout << "Belum ada log.\n";
    return;
  }

  if (logs == nullptr) {
    throw InternalGameException("Data log tidak valid.");
  }

  for (std::size_t i = 0; i < count; ++i) {
    std::cout << formatEntry(logs[i]) << '\n';
  }
}

void LogView::showLastLogs(const LogEntry *logs, std::size_t count,
                           int n) const {
  if (n <= 0) {
    std::cout << "Jumlah log harus lebih dari 0.\n";
    return;
  }

  std::cout << "=== Log Transaksi (" << n << " Terakhir) ===\n";

  if (count == 0) {
    std::cout << "Belum ada log.\n";
    return;
  }

  if (logs == nullptr) {
    throw InternalGameException("Data log tidak valid.");
  }

  std::size_t visibleCount =
      std::min<std::size_t>(count, static_cast<std::size_t>(n));
  std::size_t start = count - visibleCount;

  for (std::size_t i = start; i < count; ++i) {
    std::cout << formatEntry(logs[i]) << '\n';
  }
}
