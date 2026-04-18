#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Player.hpp"
using namespace std;

class JailManager{
    private :
        map<Player, int> jailTurns;
        int maxJailTurns;
        int jailFine;
        vector<Player> prisoners;
    public :
        JailManager(int maxJailTurns, int jailFIne);
        void sendToJail(Player& player);
        void releasePlayer(Player& player);
        void incrementJailTurn();
        bool canReleaseByFine(Player& player);
        bool canReleaseByDouble(Player& player);
        bool mustPayFine(Player& player);
        int getJailTurns(Player& player);
        bool payFine(Player& player);
        bool isInJail(Player& player);
};