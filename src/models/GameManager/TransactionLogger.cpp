#include "../../../include/models/GameManager/TransactionLogger.hpp"
#include <algorithm>


void TransactionLogger::log(int turn, const string& username, const string& action, const string& detail) {
    LogEntry newEntry(turn, username, action, detail);
    entries.push_back(newEntry);
}

void TransactionLogger::printAll() const {
    if (entries.empty()) {
        cout << "Belum ada transaksi/log yang tercatat.\n";
        return;
    }

    cout << "=== SEMUA LOG TRANSAKSI ===\n";
    for (const LogEntry& entry : entries) {
        cout << "[Turn " << entry.turn << "] Name : " << entry.username << " Action : " << entry.actionType << " Detail : " << entry.detail << "\n";
    }
}
void TransactionLogger::printLast(int n) const {
    if (entries.empty()) {
        cout << "Belum ada transaksi/log yang tercatat.\n";
        return;
    }

    if (n <= 0) {
        return;
    }
    if (static_cast<std::size_t>(n) >= entries.size()) {
        printAll();
        return;
    }
    TransactionLogger tempLogger;
    tempLogger.entries.assign(entries.end() - n, entries.end());
    cout << "=== " << n << " LOG TRANSAKSI TERAKHIR ===\n";
    tempLogger.printAll(); 
}

vector<LogEntry> TransactionLogger::getEntries() const {
    return entries;
}

void TransactionLogger::clear() {
    entries.clear();
}
