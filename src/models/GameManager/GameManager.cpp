#include "models/GameManager/GameManager.hpp"
#include "exception/NimonspoliExceptions.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "views/InputHandler.hpp"
#include "models/Command.hpp"
#include "models/GameManager/Auction.hpp"
#include "models/GameManager/BankruptcyHandler.hpp"
#include "models/Property/Property.hpp"
#include "models/Property/Street.hpp"
#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <limits>
#include <sstream>
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
  currentPlayer.resetTurn();
  tickFestivalEffects(currentPlayer);

  if (currentPlayer.isJailed()) {
    return;
  }
}

void GameManager::processCommand(string cmd) {
  if (cmd == "info") {
    cout << getCurrentPlayer().getUsername() << "\n";
  }
}

bool GameManager::checkWinCondition() const {
  if (players.empty()) {
    return false;
  }

  int aliveCount = 0;
  for (const Player &p : players) {
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

bool GameManager::isGameOver() const { return checkWinCondition(); }

Player &GameManager::getWinner() {
  int winnerIndex = -1;
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].getStatus() != BANKRUPT) {
      if (winnerIndex < 0 || players[i] > players[winnerIndex]) {
        winnerIndex = i;
      }
    }
  }

  if (winnerIndex < 0) {
    throw InvalidTurnStateException("Tidak ada pemenang - semua pemain telah bangkrut");
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
  if (newPos < 0) {
    newPos += boardSize;
  }

  movePlayerTo(player, newPos, true);
  addLogEntry("Bergerak ke petak " + to_string(newPos));
}

void GameManager::movePlayerTo(Player &player, int targetPosition,
                               bool grantGoSalary) {
  const int boardSize = getBoardSize();
  if (boardSize <= 0) {
    throw InvalidBoardPositionException(targetPosition, boardSize);
  }

  const int oldPosition = player.getPosition();
  int normalizedTarget = targetPosition % boardSize;
  if (normalizedTarget < 0) {
    normalizedTarget += boardSize;
  }

  // Validasi bahwa normalizedTarget dalam range yang valid
  if (normalizedTarget < 0 || normalizedTarget >= boardSize) {
    throw InvalidBoardPositionException(targetPosition, boardSize);
  }

  player.setPosition(normalizedTarget);
  const bool targetIsGoToJail =
      board != nullptr &&
      board->getTile(normalizedTarget).getType() == "go_to_jail";
  if (grantGoSalary &&
      !targetIsGoToJail &&
      crossesOrLandsOnGo(oldPosition, normalizedTarget)) {
    executeSalary(player, getGoSalary());
  }
}

bool GameManager::crossesOrLandsOnGo(int oldPosition, int newPosition) const {
  const int boardSize = getBoardSize();
  if (boardSize <= 0 || oldPosition == newPosition) {
    return false;
  }

  const int goPosition = board != nullptr ? board->findGoPosition() : 0;
  if (newPosition == goPosition) {
    return true;
  }

  if (oldPosition < newPosition) {
    return goPosition > oldPosition && goPosition <= newPosition;
  }

  return goPosition > oldPosition || goPosition <= newPosition;
}

int GameManager::applyDiscount(const Player &player, int amount) const {
  if (amount <= 0 || !player.hasDiscount()) {
    return amount;
  }

  const int discount = std::clamp(player.getDiscountPercentage(), 0, 100);
  return amount - (amount * discount / 100);
}

void GameManager::executePurchase(Player &player, Property &prop) {
  const int basePrice = prop.getBuyPrice();
  const int price = applyDiscount(player, basePrice);

  cout << "Kamu mendarat di " << prop.getName() << " (" << prop.getCode()
       << ")!\n";

  if (prop.getOwner() != nullptr || prop.getStatus() != PropertyStatus::BANK) {
    cout << "Properti ini sudah tidak tersedia untuk dibeli.\n";
    return;
  }

  const bool automaticPurchase =
      prop.getType() == "Railroad" || prop.getType() == "Utility";

  if (!player.canPay(price)) {
    try {
      player.ensureCanPay(price);
    } catch (const std::exception &e) {
      cout << e.what() << "\n";
    }
    cout << "Uang kamu saat ini: M" << player.getCash() << "\n";
    cout << "Properti ini akan masuk ke sistem lelang...\n";
    addLogEntry("Gagal membeli " + prop.getCode());
    executeAuction(prop);
    return;
  }

  if (!automaticPurchase) {
    cout << "Harga beli : M" << basePrice << "\n";
    if (price != basePrice) {
      cout << "Harga setelah diskon : M" << price << "\n";
    }
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

  if (payer.hasShieldActive()) {
    cout << "[SHIELD ACTIVE]: Efek ShieldCard melindungi Anda!\n";
    cout << "Tagihan sewa dibatalkan. Uang Anda tetap: M" << payer.getCash()
         << ".\n";
    logger.log(currentTurn, payer.getUsername(), "SHIELD",
               "ShieldCard membatalkan sewa " + prop.getCode());
    return;
  }

  int effectiveAmount = amount;
  effectiveAmount = applyDiscount(payer, effectiveAmount);

  try {
    payer.ensureCanPay(effectiveAmount);
    const int payerBefore = payer.getCash();
    const int ownerBefore = owner.getCash();
    payer.reduceCash(effectiveAmount);
    owner.addCash(effectiveAmount);
    cout << "Sewa         : M" << amount << "\n";
    if (effectiveAmount != amount) {
      cout << "Sewa setelah diskon : M" << effectiveAmount << "\n";
    }
    cout << "Uang kamu     : M" << payerBefore << " -> M" << payer.getCash()
         << "\n";
    cout << "Uang " << owner.getUsername() << " : M" << ownerBefore
         << " -> M" << owner.getCash() << "\n";
    logger.log(currentTurn, payer.getUsername(), "SEWA",
               "Bayar M" + to_string(effectiveAmount) + " ke " + owner.getUsername() +
                   " (" + prop.getCode() + ")");
  } catch (const std::exception &e) {
    cout << "Kamu tidak mampu membayar sewa penuh! (M" << effectiveAmount << ")\n";
    cout << e.what() << "\n";
    cout << "Uang kamu saat ini: M" << payer.getCash() << "\n";
    BankruptcyHandler bh(payer, &owner, effectiveAmount);
    if (bh.initiateLiquidation()) {
      payer.reduceCash(effectiveAmount);
      owner.addCash(effectiveAmount);
      addLogEntry("Sewa dibayar setelah likuidasi");
    } else {
      executeBankruptcy(payer, &owner, effectiveAmount);
    }
  }
}

void GameManager::executeAuction(Property &prop) {
  cout << "Properti " << prop.getName() << " (" << prop.getCode()
       << ") akan dilelang!\n";
  logger.log(currentTurn, getCurrentPlayer().getUsername(), "LELANG",
             prop.getCode() + " masuk lelang");

  if (players.empty() || prop.getOwner() != nullptr ||
      prop.getStatus() != PropertyStatus::BANK) {
    cout << "Lelang tidak dapat dimulai untuk properti ini.\n";
    return;
  }

  vector<Player *> participants;
  participants.reserve(players.size());
  for (size_t offset = 1; offset <= players.size(); ++offset) {
    Player &candidate =
        players[(activePlayerIndex + offset) % players.size()];
    if (candidate.getStatus() != BANKRUPT) {
      participants.push_back(&candidate);
    }
  }

  Auction auction(&prop, participants);
  auction.start();
  InputHandler input;

  while (auction.isOpen()) {
    Player *bidder = auction.getCurrentParticipant();
    if (bidder == nullptr) {
      break;
    }

    cout << "Penawaran tertinggi saat ini: M" << auction.getCurrentBid()
         << "\n";
    const string line =
        input.readPromptLine("Aksi lelang (PASS / BID <jumlah>): ",
                             "Aksi Lelang");
    string command;
    stringstream ss(line);
    ss >> command;
    std::transform(command.begin(), command.end(), command.begin(),
                   [](unsigned char ch) {
                     return static_cast<char>(std::toupper(ch));
                   });

    if (command == "PASS") {
      auction.pass(bidder);
      continue;
    }

    if (command == "BID") {
      int amount = 0;
      if (ss >> amount) {
        auction.submitBid(bidder, amount);
      } else {
        cout << "Format BID tidak valid. Gunakan BID <jumlah>.\n";
      }
      continue;
    }

    cout << "Input lelang tidak valid. Gunakan PASS atau BID <jumlah>.\n";
  }

  Player *winner = auction.getWinner();
  const int winningBid = auction.getWinningBid();
  if (winner == nullptr || winningBid <= 0) {
    logger.log(currentTurn, getCurrentPlayer().getUsername(), "LELANG",
               prop.getCode() + " tidak terjual");
    return;
  }

  if (!winner->canPay(winningBid)) {
    cout << "Pemenang tidak mampu membayar hasil lelang.\n";
    logger.log(currentTurn, winner->getUsername(), "LELANG",
               "Gagal membayar " + prop.getCode());
    return;
  }

  winner->reduceCash(winningBid);
  winner->addProperty(&prop);
  prop.setOwner(winner);
  prop.setStatusStr("OWNED");
  cout << prop.getName() << " kini menjadi milik " << winner->getUsername()
       << ".\n";
  logger.log(currentTurn, winner->getUsername(), "LELANG",
             "Menang " + prop.getCode() + " seharga M" +
                 to_string(winningBid));
}

void GameManager::executeBankruptcy(Player &debtor, Player *creditor,
                                    int amount) {
  cout << debtor.getUsername() << " tidak dapat membayar kewajiban M" << amount
       << ".\n";
  BankruptcyHandler bh(debtor, creditor, amount);
  bh.declareBankrupt();
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

  if (player.hasShieldActive()) {
    cout << "[SHIELD ACTIVE]: Efek ShieldCard melindungi Anda!\n";
    cout << "Tagihan M" << amount << " dibatalkan. Uang Anda tetap: M"
         << player.getCash() << ".\n";
    logger.log(currentTurn, player.getUsername(), "SHIELD",
               "ShieldCard membatalkan tagihan M" + to_string(amount));
    return;
  }

  int effectiveAmount = amount;
  effectiveAmount = applyDiscount(player, effectiveAmount);

  try {
    player.ensureCanPay(effectiveAmount);
    const int before = player.getCash();
    player.reduceCash(effectiveAmount);
    cout << "Pajak sebesar M" << amount << " langsung dipotong.\n";
    if (effectiveAmount != amount) {
      cout << "Pajak setelah diskon: M" << effectiveAmount << "\n";
    }
    cout << "Uang kamu: M" << before << " -> M" << player.getCash() << "\n";
    logger.log(currentTurn, player.getUsername(), "PAJAK",
               "Membayar pajak M" + to_string(effectiveAmount));
  } catch (const std::exception &e) {
    cout << e.what() << "\n";
    BankruptcyHandler bh(player, nullptr, effectiveAmount);
    if (bh.initiateLiquidation()) {
      player.reduceCash(effectiveAmount);
      addLogEntry("Pajak dibayar setelah likuidasi");
    } else {
      bh.declareBankrupt();
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

int GameManager::executeMortgage(Player &player, Property &prop) {
  if (prop.getOwner() != &player || !prop.canBeMortgaged()) {
    return 0;
  }

  const int received = prop.mortgage();
  if (received > 0) {
    player.addCash(received);
    logger.log(currentTurn, player.getUsername(), "GADAI",
               prop.getCode() + " digadaikan");
  }
  return received;
}

int GameManager::executeRedeem(Player &player, Property &prop) {
  if (prop.getOwner() != &player ||
      prop.getStatus() != PropertyStatus::MORTGAGED) {
    return 0;
  }

  const int price = applyDiscount(player, prop.getRedeemPrice());
  if (!player.canPay(price)) {
    return 0;
  }

  player.reduceCash(price);
  prop.redeem();
  logger.log(currentTurn, player.getUsername(), "TEBUS",
             prop.getCode() + " ditebus");
  return price;
}

int GameManager::executeBuild(Player &player, Street &street) {
  if (street.getOwner() != &player || !street.canBuildNext()) {
    return 0;
  }

  const int cost = applyDiscount(player, street.getNextBuildCost());
  if (!player.canPay(cost)) {
    return 0;
  }

  player.reduceCash(cost);
  street.buildNext();
  logger.log(currentTurn, player.getUsername(), "BANGUN",
             street.getCode() + " dibangun");
  return cost;
}

std::vector<Street *> GameManager::getEligibleBuildTargets(
    const std::vector<Street *> &streets) const {
  std::vector<Street *> eligible;
  if (streets.empty()) {
    return eligible;
  }

  int minimumBuilding = std::numeric_limits<int>::max();
  bool allReadyForHotel = true;
  for (Street *street : streets) {
    if (street == nullptr || !street->canBuildNext()) {
      continue;
    }

    minimumBuilding = std::min(minimumBuilding, street->getBuildingCount());
    if (street->getBuildingCount() <
        static_cast<int>(BuildingLevel::FOUR_HOUSE)) {
      allReadyForHotel = false;
    }
  }

  if (minimumBuilding == std::numeric_limits<int>::max()) {
    return eligible;
  }

  for (Street *street : streets) {
    if (street == nullptr || !street->canBuildNext()) {
      continue;
    }

    const bool canBuild =
        allReadyForHotel
            ? street->getBuildingCount() ==
                  static_cast<int>(BuildingLevel::FOUR_HOUSE)
            : street->getBuildingCount() == minimumBuilding &&
                  street->getBuildingCount() <
                      static_cast<int>(BuildingLevel::FOUR_HOUSE);
    if (canBuild) {
      eligible.push_back(street);
    }
  }
  return eligible;
}

bool GameManager::ownsFullColorGroup(const Player &player,
                                     ColorGroup color) const {
  if (board == nullptr) {
    return false;
  }

  bool groupExists = false;
  for (int i = 0; i < board->getTileCount(); ++i) {
    Tile &tile = board->getTile(i);
    if (tile.getType() != "property") {
      continue;
    }

    PropertyTile &propertyTile = static_cast<PropertyTile &>(tile);
    Property &prop = propertyTile.getProperty();
    if (prop.getType() != "Street") {
      continue;
    }

    Street &street = static_cast<Street &>(prop);
    if (street.getColorGroup() != color) {
      continue;
    }

    groupExists = true;
    if (street.getOwner() != &player) {
      return false;
    }
  }
  return groupExists;
}

int GameManager::sellBuildingsInColorGroup(Player &player, ColorGroup color) {
  int totalReceived = 0;
  for (Property *prop : player.getProperties()) {
    if (prop == nullptr || prop->getType() != "Street") {
      continue;
    }

    Street *street = static_cast<Street *>(prop);
    if (street->getColorGroup() != color || street->getBuildingCount() <= 0) {
      continue;
    }

    const int received = street->getBuildingSellValue();
    street->demolish();
    player.addCash(received);
    totalReceived += received;
    logger.log(currentTurn, player.getUsername(), "JUAL_BANGUNAN",
               street->getCode() + " bangunan dijual M" + to_string(received));
  }
  return totalReceived;
}

bool GameManager::hasBuildingsInColorGroup(const Player &player,
                                           ColorGroup color) const {
  for (Property *prop : player.getProperties()) {
    if (prop == nullptr || prop->getType() != "Street") {
      continue;
    }

    const Street *street = static_cast<const Street *>(prop);
    if (street->getColorGroup() == color && street->getBuildingCount() > 0) {
      return true;
    }
  }
  return false;
}

void GameManager::destroyProperty(Player &actor, Property &prop) {
  Player *owner = prop.getOwner();
  if (owner == nullptr || owner == &actor) {
    return;
  }

  owner->removeProperty(&prop);
  prop.resetToBank();
  logger.log(currentTurn, actor.getUsername(), "KARTU",
             "DemolitionCard menghancurkan " + prop.getCode());
}

void GameManager::tickFestivalEffects(Player &owner) {
  if (board == nullptr) {
    return;
  }

  for (int i = 0; i < board->getTileCount(); ++i) {
    Tile &tile = board->getTile(i);
    if (tile.getType() != "property") {
      continue;
    }

    PropertyTile &propertyTile = static_cast<PropertyTile &>(tile);
    Property &prop = propertyTile.getProperty();
    if (prop.getOwner() == &owner) {
      prop.tickFestival();
    }
  }
}

void GameManager::visitJail(Player &player) {
  cout << player.getUsername() << " sedang mampir di Penjara.\n";
}

void GameManager::goToJail(Player &player) {
  if (player.hasShieldActive()) {
    cout << "[SHIELD ACTIVE]: Efek ShieldCard melindungi Anda!\n";
    cout << "Sanksi masuk penjara dibatalkan.\n";
    logger.log(currentTurn, player.getUsername(), "SHIELD",
               "ShieldCard membatalkan sanksi penjara");
    return;
  }

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
