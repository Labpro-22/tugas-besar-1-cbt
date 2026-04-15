#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "Player.hpp"
using namespace std;

class LogEntry {
public:
    int turn;
    string username;
    string actionType;
    string detail;
    LogEntry(int turn, string username, string actionType, string detail);
};
