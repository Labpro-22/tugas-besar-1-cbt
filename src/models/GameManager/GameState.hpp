#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "TransactionLogger.hpp"
#include "Player.hpp"
using namespace std;


class GameState {
private:
    int currentTurn;
    int maxTurn;
    int activePlayerIndex;
    vector<Player> players;
    vector<int> turnOrder;
    vector<Property> properties;
    vector<SkillCard*> skillDeckCards;
    vector<LogEntry> log;

public:
    string serialize() const;
    void deserialize(const string& data);
};