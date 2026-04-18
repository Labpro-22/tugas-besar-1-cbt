#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "Player.hpp"
#include "Dice.hpp"
#include "JailManager.hpp"
#include "TransactionLogger.hpp"
#include "FestivalEffect.hpp"
#include "../Property/Property.hpp"
#include "../Card/CardDeck.hpp"
using namespace std;

class GameManager {
    private :
        int currentTurn;
        int maxTurn;
        int activePlayerIndex;
        vector<Player> players;
        vector<int> turnOrder;
        Dice dice;
        CardDeck<Card*> chanceDeck;
        CardDeck<Card*> chestDeck;
        TransactionLogger logger;
        FestivalEffect festivalEffect;
        JailManager jailManager;
    public :
        void startNewGame();
        void processTurn();
        void processCommand(string cmd);
        bool checkWinCondition();
        Player& getCurrentPlayer();
        void advanceToNextPlayer();
        bool isGameOver();
        Player& getWinner();
        void moveCurrentPlayer(int steps);
        void movePlayerTo(Player& player, int position);
        void sendPlayerToJail(Player& player);
        int getNearestStationPosition(int position) const;
        bool executePurchase(Player& player, Property& prop);
        void executeRentPayer(Player&, Player& owner, Property& prop, int amount);
        void executeAuction(Property& prop);
        void executeBankruptcy(Player& debtor, Player* creditor, int amount);
        void executeFestival(Player& player, string propCode);
        void executeTaxPayment(Player& player, int amount, bool toBank);
        void executeChanceCard(Player& player, Card& card);
        void executeCommunityChestCard(Player& player, Card& card);
        void addLogEntry(string action);
        vector<Player>& getPlayers() { return players; }
        const vector<Player>& getPlayers() const { return players; }
        CardDeck<Card*>& getChanceDeck() { return chanceDeck; }
        CardDeck<Card*>& getCommunityChestDeck() { return chestDeck; }
};