#pragma once
#include <iostream>
#include "Player.hpp"
#include "Dice.hpp"
#include "JailManager.hpp"
#include "TransactionLogger.hpp"
#include "FestivalEffect.hpp"
#include "../Card/CardDeck.hpp"
#include ""
#include <vector>
using namespace std;

class GameManager {
    private :
        int currentTurn;
        int maxTurn;
        int activePlayerIndex;
        vector<Player> players;
        vector<int> turnOrder;
        Board board;
        Dice dice;
        CardDeck<ChanceCard> chanceDeck;
        CardDeck<CommunityChestCard> chestDeck;
        CardDeck skillDeck;
        DiscardPile discardPile;
        Configuration config;
        TransactionLogger logger;
        FestivalEffect festivalEffect;
        JailManager jailManager;
        Dice dice;
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
        void executePurchase(Player& player, Property& prop);
        void executeRentPayer(Player&, Player& owner, Property& prop, int amount);
        void executeAuction(Property& prop);
        void executeBankruptcy(Player& debtor, Player& creditor, int amount);
        void executeFestival(Player& player, string propCode);
        void executeTaxPayment(Player& player, int amount, bool toBank);
        void addLogEntry(string action);
        vector<Player>& getPlayers() { return players; }
        const vector<Player>& getPlayers() const { return players; }
};