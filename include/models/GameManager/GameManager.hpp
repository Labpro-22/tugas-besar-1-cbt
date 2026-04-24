#pragma once
#include "data/Configuration.hpp"
#include "../Card/CardDeck.hpp"
#include "../Card/ChanceCard.hpp"
#include "../Card/CommunityCard.hpp"
#include "../Card/SkillCard.hpp"
#include "../Property/ColorGroup.hpp"
#include "Dice.hpp"
#include "Player.hpp"
#include "TransactionLogger.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Board;
class Street;
class Property;

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
  Configuration config;
  TransactionLogger logger;
  std::function<void()> snapshotRefreshFn;

public:
  GameManager();
  void startNewGame();
  void processTurn();
  void processCommand(string cmd);
  bool checkWinCondition() const;
  Player &getCurrentPlayer();
  const Player &getCurrentPlayer() const;
  void advanceToNextPlayer();
  bool isGameOver() const;
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
  void movePlayerTo(Player &player, int targetPosition, bool grantGoSalary);
  bool crossesOrLandsOnGo(int oldPosition, int newPosition) const;
  int applyDiscount(const Player &player, int amount) const;
  void executePurchase(Player &player, Property &prop);
  void executeRentPayer(Player &, Player &owner, Property &prop, int amount);
  void executeAuction(Property &prop);
  void executeBankruptcy(Player *debtor, Player *creditor, int amount);
  void executeFestival(Player &player, string propCode);
  void executeTaxPayment(Player &player, int amount, bool toBank);
  void executeSalary(Player &player, int amount);
  int executeMortgage(Player &player, Property &prop);
  int executeRedeem(Player &player, Property &prop);
  int executeBuild(Player &player, Street &street);
  std::vector<Street *> getEligibleBuildTargets(const std::vector<Street *> &streets) const;
  bool ownsFullColorGroup(const Player &player, ColorGroup color) const;
  bool anyPropertyInGroupMortgaged(const Player &player, ColorGroup color) const;
  int sellBuildingsInColorGroup(Player &player, ColorGroup color);
  bool hasBuildingsInColorGroup(const Player &player, ColorGroup color) const;
  void destroyProperty(Player &actor, Property &prop);
  void tickFestivalEffects(Player &owner);
  void goToJail(Player &player);
  void setSnapshotRefreshCallback(std::function<void()> fn) { snapshotRefreshFn = std::move(fn); }
  void pushSnapshot() { if (snapshotRefreshFn) snapshotRefreshFn(); }
  Board &getBoard();
  TransactionLogger &getLogger();
  const TransactionLogger &getLogger() const;

  // Dynamic board accessors for card/tile logic
  int getGoSalary() const;
  int getJailPosition() const;
  int getBoardSize() const;
};
