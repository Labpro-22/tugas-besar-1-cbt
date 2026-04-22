#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../Property/Property.hpp"
#include "../Card/SkillCardFactory.hpp"
#include "Player.hpp"
#include "TransactionLogger.hpp"
using namespace std;

class GameState {
private:
    int currentTurn;
    int maxTurn;
    int activePlayerIndex;
    vector<Player> players;
    vector<int> turnOrder;
    vector<Property *> properties;
    vector<SkillCard *> skillDeckCards;
    vector<LogEntry> log;
    SkillCardFactory skillCardFactory;

public:
    GameState(int currentTurn = 0, int maxTurn = 0, int activePlayerIndex = 0,
        vector<Player> players = {}, vector<int> turnOrder = {},
        vector<Property *> properties = {},
        vector<SkillCard *> skillDeckCard = {}, vector<LogEntry> log = {});

    string serialize() const;
    void deserialize(const string &data);
};
