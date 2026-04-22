#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class LogEntry {
public:
    int turn;
    string username;
    string actionType;
    string detail;
    LogEntry(int turn, string username, string actionType, string detail);
};
