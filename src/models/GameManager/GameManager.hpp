#pragma once
#include "../../placeholder/Configuration.hpp"
#include "../../placeholder/DiscardPile.hpp"
#include "../Card/CardDeck.hpp"
#include "../Card/ChanceCard.hpp"
#include "../Card/CommunityCard.hpp"
#include "../Card/SkillCard.hpp"
#include "Dice.hpp"
#include "FestivalEffect.hpp"
#include "Player.hpp"
#include "TransactionLogger.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// Forward declarations
class Board;

class GameManager {
private:
    int currentTurn;
    int maxTurn;
    int activePlayerIndex;
    vector<Player> players;
    vector<int> turnOrder;
    Board *board;
    Dice dice;
    CardDeck<ChanceCard> chanceDeck;
    CardDeck<CommunityCard> chestDeck;
    CardDeck<SkillCard> skillDeck;
    DiscardPile discardPile;
    Configuration config;
    TransactionLogger logger;
    FestivalEffect festivalEffect;

public:
    GameManager();
    void startNewGame();
    void processTurn();
    void processCommand(string cmd);
    bool checkWinCondition();
    Player &getCurrentPlayer();
    void advanceToNextPlayer();
    bool isGameOver();
    Player &getWinner();
    vector<Player> &getPlayers();
    void moveCurrentPlayer(int steps);
    void executePurchase(Player &player, Property &prop);
    void executeRentPayer(Player &, Player &owner, Property &prop, int amount);
    void executeAuction(Property &prop);
    void executeBankruptcy(Player &debtor, Player *creditor, int amount);
    void executeFestival(Player &player, string propCode);
    void executeTaxPayment(Player &player, int amount, bool toBank);
    void addLogEntry(string action);
    Board &getBoard();
};