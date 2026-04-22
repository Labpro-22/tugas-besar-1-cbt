#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "LogEntry.hpp"
using namespace std;


class TransactionLogger {
private:
    vector<LogEntry> entries;

public:
    void log(int turn, const string& username, const string& action, const string& detail);
    void printAll() const;
    void printLast(int n) const;
    vector<LogEntry> getEntries() const;
    void clear();
};

