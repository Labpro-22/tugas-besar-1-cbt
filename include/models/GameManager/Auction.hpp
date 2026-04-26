#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Player.hpp"
using namespace std;


class Auction{
    private :
        Property* property;
        vector<Player*> participants;
        map<string, int> bids;
        int currentBid;
        int minimumBid;
        int winnerIndex;
        int passCount;
        int currentParticipantIndex;
        bool isActive;
        void determineWinner();
        void nextParticipant();
    public :
        Auction(Property* prop, vector<Player*> participants);
        void start();
        bool submitBid(Player* player, int amount);
        void pass(Player* player);
        Player* getWinner();
        int getWinningBid();
        bool isOpen() const;
        Player* getCurrentParticipant() const;
        int getCurrentBid() const;
        int getMinimumBid() const;
};
