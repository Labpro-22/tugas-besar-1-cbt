#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class Player;

class JailManager {
private:
    map<string, int> jailTurns;
    int maxJailTurns;
    int jailFine;
    vector<Player *> prisoners;

public:
    JailManager();
    JailManager(int maxJailTurns, int jailFine);
    void sendToJail(Player &player);
    void releasePlayer(Player &player);
    void incrementJailTurn();
    bool canReleaseByFine(Player &player);
    bool canReleaseByDouble(Player &player);
    bool mustPayFine(Player &player);
    int getJailTurns(Player &player);
    bool payFine(Player &player);
    bool isInJail(Player &player);
};