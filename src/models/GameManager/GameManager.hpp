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
  const Player &getCurrentPlayer() const;
  void advanceToNextPlayer();
  bool isGameOver();
  Player &getWinner();
  vector<Player> &getPlayers();
  const vector<Player> &getPlayers() const;
  int getCurrentTurn() const;
  int getMaxTurn() const;
  int getActivePlayerIndex() const;
  void setCurrentTurn(int turn);
  void setActivePlayerIndex(int index);
  void setMaxTurn(int turns);
  void setBoard(Board *newBoard);
  void setConfiguration(const Configuration &newConfig);
  void moveCurrentPlayer(int steps);
  void executePurchase(Player &player, Property &prop);
  void executeRentPayer(Player &, Player &owner, Property &prop, int amount);
  void executeAuction(Property &prop);
  void executeBankruptcy(Player &debtor, Player *creditor, int amount);
  void executeFestival(Player &player, string propCode);
  void executeTaxPayment(Player &player, int amount, bool toBank);
  void executeSalary(Player &player, int amount);
  void visitJail(Player &player);
  void goToJail(Player &player);
  void addLogEntry(string action);
  Board &getBoard();
  TransactionLogger &getLogger();
  const TransactionLogger &getLogger() const;

  // Dynamic board accessors for card/tile logic
  int getGoSalary() const;
  int getJailPosition() const;
  int getBoardSize() const;
};
