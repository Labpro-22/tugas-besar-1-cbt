#include "GameManager.hpp"
#include "../../core/Board-Tiles/Board.hpp"
#include "../../views/InputHandler.hpp"
#include "BankruptcyHandler.hpp"
#include "../Property/Property.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

using namespace std;

GameManager::GameManager()
    : currentTurn(0), maxTurn(0), activePlayerIndex(0), board(nullptr) {}

void GameManager::startNewGame() {
  currentTurn = 1;
  activePlayerIndex = 0;
  addLogEntry("Game Dimulai");
}

void GameManager::processTurn() {
  Player &currentPlayer = getCurrentPlayer();

  if (currentPlayer.isJailed()) {
    return;
  }

  currentPlayer.resetTurn();
}

void GameManager::processCommand(string cmd) {
  if (cmd == "info") {
    cout << getCurrentPlayer().getUsername() << "\n";
  }
}

bool GameManager::checkWinCondition() {
  if (players.empty()) {
    return false;
  }

  int aliveCount = 0;
  for (Player &p : players) {
    if (p.getStatus() != BANKRUPT) {
      aliveCount++;
    }
  }
  return (aliveCount <= 1) || (maxTurn > 0 && currentTurn > maxTurn);
}

Player &GameManager::getCurrentPlayer() { return players[activePlayerIndex]; }

const Player &GameManager::getCurrentPlayer() const {
  return players[activePlayerIndex];
}

const vector<Player> &GameManager::getPlayers() const { return players; }

int GameManager::getCurrentTurn() const { return currentTurn; }

int GameManager::getMaxTurn() const { return maxTurn; }

int GameManager::getActivePlayerIndex() const { return activePlayerIndex; }

void GameManager::setCurrentTurn(int turn) { currentTurn = max(0, turn); }

void GameManager::setActivePlayerIndex(int index) {
  if (players.empty()) {
    activePlayerIndex = 0;
    return;
  }

  if (index < 0) {
    activePlayerIndex = 0;
    return;
  }

  activePlayerIndex = index % static_cast<int>(players.size());
}

void GameManager::setMaxTurn(int turns) { maxTurn = max(0, turns); }

void GameManager::setBoard(Board *newBoard) { board = newBoard; }

void GameManager::setConfiguration(const Configuration &newConfig) {
  config = newConfig;
}

void GameManager::advanceToNextPlayer() {
  if (players.empty()) {
    return;
  }

  do {
    activePlayerIndex = (activePlayerIndex + 1) % players.size();
    if (activePlayerIndex == 0) {
      currentTurn++;
    }
  } while (players[activePlayerIndex].getStatus() == BANKRUPT);
}

bool GameManager::isGameOver() { return checkWinCondition(); }

Player &GameManager::getWinner() {
  int maxWealth = -1;
  int winnerIndex = 0;
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].getStatus() != BANKRUPT) {
      int wealth = players[i].getTotalWealth();
      if (wealth > maxWealth) {
        maxWealth = wealth;
        winnerIndex = i;
      }
    }
  }
  return players[winnerIndex];
}

vector<Player> &GameManager::getPlayers() { return players; }

void GameManager::moveCurrentPlayer(int steps) {
  Player &player = getCurrentPlayer();
  int oldPos = player.getPosition();
  int boardSize = getBoardSize();

  if (boardSize <= 0) {
    return;
  }

  int newPos = (oldPos + steps) % boardSize;

  player.setPosition(newPos);

  if (newPos < oldPos || newPos == 0) {
    executeSalary(player, getGoSalary());
  }

  addLogEntry("Bergerak ke petak " + to_string(newPos));
}

void GameManager::executePurchase(Player &player, Property &prop) {
  int price = prop.getBuyPrice();

  cout << "Kamu mendarat di " << prop.getName() << " (" << prop.getCode()
       << ")!\n";

  if (prop.getOwner() != nullptr || prop.getStatus() != PropertyStatus::BANK) {
    cout << "Properti ini sudah tidak tersedia untuk dibeli.\n";
    return;
  }

  const bool automaticPurchase =
      prop.getType() == "Railroad" || prop.getType() == "Utility";

  if (!player.canPay(price)) {
    cout << "Uang kamu saat ini: M" << player.getCash() << "\n";
    cout << "Properti ini akan masuk ke sistem lelang...\n";
    addLogEntry("Gagal membeli " + prop.getCode());
    executeAuction(prop);
    return;
  }

  if (!automaticPurchase) {
    cout << "Harga beli : M" << price << "\n";
    cout << "Uang kamu  : M" << player.getCash() << "\n";
    InputHandler input;
    const bool shouldBuy = input.readYesNo(
        "Apakah kamu ingin membeli properti ini? (y/n): ");

    if (!shouldBuy) {
      cout << "Properti ini akan masuk ke sistem lelang...\n";
      addLogEntry("Menolak membeli " + prop.getCode());
      executeAuction(prop);
      return;
    }
  } else {
    cout << "Belum ada yang menginjaknya duluan, " << prop.getName()
         << " kini menjadi milikmu!\n";
  }

  player.reduceCash(price);
  player.addProperty(&prop);
  prop.setOwner(&player);
  prop.setStatusStr("OWNED");

  cout << prop.getName() << " kini menjadi milikmu!\n";
  cout << "Uang tersisa: M" << player.getCash() << "\n";
  logger.log(currentTurn, player.getUsername(), "BELI",
             "Beli " + prop.getCode() + " seharga M" + to_string(price));
}

void GameManager::executeRentPayer(Player &payer, Player &owner, Property &prop,
                                   int amount) {
  cout << "Kamu mendarat di " << prop.getName() << " (" << prop.getCode()
       << "), milik " << owner.getUsername() << "!\n";

  if (payer.canPay(amount)) {
    const int payerBefore = payer.getCash();
    const int ownerBefore = owner.getCash();
    payer.reduceCash(amount);
    owner.addCash(amount);
    cout << "Sewa         : M" << amount << "\n";
    cout << "Uang kamu     : M" << payerBefore << " -> M" << payer.getCash()
         << "\n";
    cout << "Uang " << owner.getUsername() << " : M" << ownerBefore
         << " -> M" << owner.getCash() << "\n";
    logger.log(currentTurn, payer.getUsername(), "SEWA",
               "Bayar M" + to_string(amount) + " ke " + owner.getUsername() +
                   " (" + prop.getCode() + ")");
  } else {
    cout << "Kamu tidak mampu membayar sewa penuh! (M" << amount << ")\n";
    cout << "Uang kamu saat ini: M" << payer.getCash() << "\n";
    BankruptcyHandler bh(payer, &owner, amount);
    if (bh.initiateLiquidation()) {
      payer.reduceCash(amount);
      owner.addCash(amount);
      addLogEntry("Sewa dibayar setelah likuidasi");
    } else {
      executeBankruptcy(payer, &owner, amount);
    }
  }
}

void GameManager::executeAuction(Property &prop) {
  cout << "Properti " << prop.getName() << " (" << prop.getCode()
       << ") akan dilelang!\n";
  logger.log(currentTurn, getCurrentPlayer().getUsername(), "LELANG",
             prop.getCode() + " masuk lelang");
}

void GameManager::executeBankruptcy(Player &debtor, Player *creditor,
                                    int amount) {
  cout << debtor.getUsername() << " tidak dapat membayar kewajiban M" << amount
       << ".\n";
  BankruptcyHandler bh(debtor, creditor, amount);
  bh.declareBankrupt();
  bh.transferAssets();
  if (creditor) {
    cout << debtor.getUsername() << " dinyatakan BANGKRUT!\n";
    cout << "Kreditor: " << creditor->getUsername() << "\n";
    addLogEntry("Bangkrut dan aset disita oleh " + creditor->getUsername());
  } else {
    cout << debtor.getUsername() << " dinyatakan BANGKRUT!\n";
    cout << "Kreditor: Bank\n";
    addLogEntry("Bangkrut ke Bank");
  }
}

void GameManager::executeFestival(Player &, string propCode) {
  addLogEntry("Mengadakan festival di " + propCode);
}

void GameManager::executeTaxPayment(Player &player, int amount, bool toBank) {
  (void)toBank;

  if (player.canPay(amount)) {
    const int before = player.getCash();
    player.reduceCash(amount);
    cout << "Pajak sebesar M" << amount << " langsung dipotong.\n";
    cout << "Uang kamu: M" << before << " -> M" << player.getCash() << "\n";
    logger.log(currentTurn, player.getUsername(), "PAJAK",
               "Membayar pajak M" + to_string(amount));
  } else {
    BankruptcyHandler bh(player, nullptr, amount);
    if (bh.initiateLiquidation()) {
      player.reduceCash(amount);
      addLogEntry("Pajak dibayar setelah likuidasi");
    } else {
      bh.declareBankrupt();
      bh.repossessProperties();
      addLogEntry("Bangkrut karena pajak");
    }
  }
}

void GameManager::executeSalary(Player &player, int amount) {
  player.addCash(amount);
  cout << player.getUsername() << " menerima gaji GO sebesar M" << amount
       << ".\n";
  logger.log(currentTurn, player.getUsername(), "GO",
             "Menerima gaji M" + to_string(amount));
}

void GameManager::visitJail(Player &player) {
  cout << player.getUsername() << " sedang mampir di Penjara.\n";
}

void GameManager::goToJail(Player &player) {
  player.setPosition(getJailPosition());
  player.setStatus(JAILED);
  cout << player.getUsername() << " dikirim ke Penjara.\n";
  logger.log(currentTurn, player.getUsername(), "PENJARA",
             "Masuk penjara");
}

void GameManager::addLogEntry(string action) {
  if (players.empty()) {
    logger.log(currentTurn, "-", action, "");
    return;
  }

  string username = getCurrentPlayer().getUsername();
  logger.log(currentTurn, username, action, "");
}

Board &GameManager::getBoard() { return *board; }

TransactionLogger &GameManager::getLogger() { return logger; }

const TransactionLogger &GameManager::getLogger() const { return logger; }

int GameManager::getGoSalary() const { return config.getGoSalary(); }

int GameManager::getJailPosition() const {
  if (board != nullptr) {
    return board->findJailPosition();
  }
  return 10; // Fallback default
}

int GameManager::getBoardSize() const {
  if (board != nullptr) {
    return board->getTileCount();
  }
  return 40; // Fallback default
}
