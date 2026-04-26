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

GameManager::GameManager() : currentTurn(0), maxTurn(0), activePlayerIndex(0), lastDiceTotal(0), board(nullptr) {}

void GameManager::startNewGame() {
  currentTurn = 1;
  activePlayerIndex = 0;
  logger.log(currentTurn, "-", "MULAI", "Game Dimulai");
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

bool GameManager::anyPropertyInGroupMortgaged(const Player &player, ColorGroup color) const {
  if (board == nullptr) {
    return false;
  }

  for (int i = 0; i < board->getTileCount(); ++i) {
    Tile &tile = board->getTile(i);
    if (tile.getType() != "property") {
      continue;
    }

    PropertyTile &propertyTile = static_cast<PropertyTile &>(tile);
    Property &prop = propertyTile.getProperty();
    if (prop.getOwner() == &player) {
      bool isTargetGroup = false;
      if (prop.getType() == "Street") {
        isTargetGroup = (static_cast<Street &>(prop).getColorGroup() == color);
      }

      if (isTargetGroup && prop.getStatus() == PropertyStatus::MORTGAGED) {
        return true;
      }
    }
  }

  return false;
}

Player &GameManager::getCurrentPlayer() { return players[activePlayerIndex]; }

const Player &GameManager::getCurrentPlayer() const {
  return players[activePlayerIndex];
}

const vector<Player> &GameManager::getPlayers() const { return players; }

vector<Player> &GameManager::getPlayers() {
  return const_cast<vector<Player> &>(
      static_cast<const GameManager *>(this)->getPlayers());
}

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
  int aliveCount = 0;
  for (const Player& p : players) {
    if (p.getStatus() != BANKRUPT) {
      aliveCount++;
    }
  }
  if (aliveCount <= 1) {
    return;
  }
  int startIdx = activePlayerIndex;
  do {
    activePlayerIndex = (activePlayerIndex + 1) % players.size();
    if (activePlayerIndex == 0) {
      currentTurn++;
    }
    if (activePlayerIndex == startIdx && players[activePlayerIndex].getStatus() == BANKRUPT) {
        break;
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
}

void GameManager::movePlayerTo(Player &player, int targetPosition, bool grantGoSalary) {
  const int boardSize = getBoardSize();
  if (boardSize <= 0) {
    throw InvalidBoardPositionException(targetPosition, boardSize);
  }

  const int oldPosition = player.getPosition();
  int normalizedTarget = targetPosition % boardSize;
  if (normalizedTarget < 0) {
    normalizedTarget += boardSize;
  }

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
    player.incrementLap();
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
    logger.log(currentTurn, player.getUsername(), "BELI",
               "Tidak mampu membeli " + prop.getCode() + ", masuk lelang");
    executeAuction(prop);
    return;
  }

  if (!automaticPurchase) {
    cout << "Harga beli : M" << basePrice << "\n";
    if (price != basePrice) {
      cout << "Harga setelah diskon : M" << price << "\n";
    }
    cout << "Uang kamu  : M" << player.getCash() << "\n";
    cout << "Uang tersisa jika dibeli: M" << (player.getCash() - price) << "\n";
    pushSnapshot();
    InputHandler input;
    const bool shouldBuy = input.readYesNo(
        "[PROPERTY_PURCHASE:" + prop.getCode() + "] Apakah kamu ingin membeli properti ini? (y/n): ");

    if (!shouldBuy) {
      cout << "Properti ini akan masuk ke sistem lelang...\n";
      logger.log(currentTurn, player.getUsername(), "BELI",
                 "Menolak beli " + prop.getCode() + ", masuk lelang");
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
  logger.log(currentTurn, player.getUsername(), "BELI", "Beli " + prop.getName() + " (" + prop.getCode() + ") seharga M" +
            to_string(price) + ". Uang tersisa: M" + to_string(player.getCash()));
}

void GameManager::executeRentPayer(Player &payer, Player &owner, Property &prop, int amount) {
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
      logger.log(currentTurn, payer.getUsername(), "SEWA",
                 "Bayar M" + to_string(effectiveAmount) + " ke " +
                     owner.getUsername() + " (" + prop.getCode() +
                     ") setelah likuidasi");
    } else {
      executeBankruptcy(&payer, &owner, effectiveAmount);
    }
  }
}

void GameManager::executeAuction(Property &prop) {
    cout << "Properti " << prop.getName() << " (" << prop.getCode() << ") akan dilelang!\n";
    logger.log(currentTurn, getCurrentPlayer().getUsername(), "LELANG", prop.getCode() + " masuk lelang");

    if (players.empty() || prop.getOwner() != nullptr || prop.getStatus() != PropertyStatus::BANK) {
        cout << "Lelang tidak dapat dimulai untuk properti ini.\n";
        return;
    }

    vector<Player *> participants;
    int nPlayers = players.size();
    for (int i = 1; i < nPlayers; ++i) {
      int targetIdx = (activePlayerIndex + i) % nPlayers;
      if (players[targetIdx].getStatus() != BANKRUPT) {
        participants.push_back(&players[targetIdx]);
      }
    }
    participants.push_back(&players[activePlayerIndex]);
    if (participants.empty()) {
        cout << "Tidak ada peserta lelang.\n";
        return;
    }
    Auction auction(&prop, participants);
    auction.start();
    
    InputHandler input;
    int consecutivePass = 0;
  while (auction.isOpen()) {
    Player *bidder = auction.getCurrentParticipant();
    
    bool mustBid = (consecutivePass == (int)participants.size() - 1 && auction.getWinningBid() == 0);
    if (mustBid && bidder->getCash() < auction.getMinimumBid()) {
      cout << "\n[KRITIS] " << bidder->getUsername() << " wajib bid tapi uang tidak cukup." << endl;
      executeBankruptcy(bidder, nullptr, auction.getMinimumBid()); 
      
      auction.pass(bidder);        
      break; 
    }
    if (bidder->getStatus() == BANKRUPT) {
            auction.pass(bidder);
            continue;
    }
    cout << "\nGiliran: " << bidder->getUsername() << " | Bid Tertinggi: M" << auction.getCurrentBid() << "\n";
    
    string prompt = mustBid ? "Aksi (WAJIB BID <jumlah>): " : "Aksi (PASS / BID <jumlah>): ";
    string line = input.readPromptLine(prompt, "Lelang");
    
    stringstream ss(line);
    string command;
    ss >> command;
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    if (command == "PASS") {
      logger.log(currentTurn, bidder->getUsername(), "PASS", prop.getCode() + " lelang");
      if (mustBid) {
        cout << "[!] Anda adalah penawar terakhir dan belum ada bid. Anda WAJIB membeli properti ini!\n";
        continue;
      }
      auction.pass(bidder);
      consecutivePass++;
    } 
    else if (command == "BID") {
      logger.log(currentTurn, bidder->getUsername(), "BID", prop.getCode() + " lelang");
      int amount;
      if (ss >> amount) {
        if (amount > bidder->getCash()) {
          cout << "[!] Uang tidak cukup! Saldo Anda: M" << bidder->getCash() 
               << ". Silakan masukkan jumlah BID yang sesuai.\n";
          continue;
        }
        if (amount < auction.getMinimumBid() || amount <= auction.getWinningBid()) {
          cout << "[!] Bid terlalu rendah! Minimal Bid: M" 
               << max(auction.getMinimumBid(), auction.getWinningBid() + 1) << ".\n";
          continue; 
        }
        if (auction.submitBid(bidder, amount)) {
          consecutivePass = 0;
        }
      } else {
        cout << "Format salah! Gunakan: BID <jumlah>\n";
      }
    }
  }

  Player *winner = auction.getWinner();
  if (winner != nullptr) {
    int price = auction.getWinningBid();
    winner->reduceCash(price);
    winner->addProperty(&prop);
    prop.setOwner(winner);
    prop.setStatusStr("OWNED");
    
    cout << "\n[PENGUMUMAN] " << winner->getUsername() << " menang lelang seharga M" << price << "!\n";
    logger.log(currentTurn, winner->getUsername(), "LELANG", "Beli " + prop.getCode());
  }else {
    cout << "\nLelang dibatalkan atau berakhir tanpa pemenang karena pemain terakhir bangkrut.\n";
  }
}

void GameManager::executeBankruptcy(Player *debtor, Player *creditor, int amount) {
  cout << debtor->getUsername() << " tidak dapat membayar kewajiban M" << amount
       << ".\n";
  BankruptcyHandler bh(*debtor, creditor, amount);
  bh.declareBankrupt();
  if (activePlayerIndex >= 0 &&
      static_cast<std::size_t>(activePlayerIndex) < players.size() &&
      players[static_cast<std::size_t>(activePlayerIndex)].getStatus() ==
          BANKRUPT) {
    advanceToNextPlayer();
  }
  if (creditor) {
    cout << debtor->getUsername() << " dinyatakan BANGKRUT!\n";
    cout << "Kreditor: " << creditor->getUsername() << "\n";
    logger.log(currentTurn, debtor->getUsername(), "BANGKRUT",
               "Aset disita oleh " + creditor->getUsername());
  } else {
    cout << debtor->getUsername() << " dinyatakan BANGKRUT!\n";
    cout << "Kreditor: Bank\n";
    logger.log(currentTurn, debtor->getUsername(), "BANGKRUT", "Bangkrut ke Bank");
  }
}

void GameManager::executeFestival(Player &player, string propCode) {
  // Logging for festival is handled entirely by FestivalTile before and after this call
  (void)player;
  (void)propCode;
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
      logger.log(currentTurn, player.getUsername(), "PAJAK",
                 "Pajak M" + to_string(effectiveAmount) + " dibayar setelah likuidasi");
    } else {
      bh.declareBankrupt();
      logger.log(currentTurn, player.getUsername(), "BANGKRUT",
                 "Bangkrut karena tidak mampu membayar pajak M" +
                     to_string(effectiveAmount));
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
  if (prop.getOwner() != &player) {
    throw PropertyMortgageException(prop.getCode(), "Anda bukan pemilik properti ini.");
  }
  if (!prop.canBeMortgaged()) {
    throw PropertyMortgageException(prop.getCode(), "Properti ini sedang digadaikan atau memiliki bangunan.");
  }
  
  // Check group building rule

  if (prop.getType() == "Street") {
    Street &street = static_cast<Street &>(prop);
    if (hasBuildingsInColorGroup(player, street.getColorGroup())) {
      throw PropertyMortgageException(prop.getCode(), "Tidak bisa menggadaikan properti jika masih ada bangunan di color group-nya.");
    }
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
  if (prop.getOwner() != &player) {
    throw PropertyRedeemException(prop.getCode(), "Anda bukan pemilik properti ini.");
  }
  if (prop.getStatus() != PropertyStatus::MORTGAGED) {
    throw PropertyRedeemException(prop.getCode(), "Properti tidak sedang digadaikan.");
  }

  const int price = applyDiscount(player, prop.getRedeemPrice());
  if (!player.canPay(price)) {
    throw InsufficientFundsException(player.getUsername(), price, player.getCash());
  }

  player.reduceCash(price);
  prop.redeem();
  logger.log(currentTurn, player.getUsername(), "TEBUS",
             prop.getCode() + " ditebus");
  return price;
}

int GameManager::executeBuild(Player &player, Street &street) {
  if (street.getOwner() != &player) {
    throw PropertyBuildException(street.getCode(), "Anda bukan pemilik properti ini.");
  }
  if (!street.canBuildNext()) {
    throw PropertyBuildException(street.getCode(), "Properti sudah mencapai level maksimal.");
  }

  const int cost = applyDiscount(player, street.getNextBuildCost());
  if (!player.canPay(cost)) {
    throw InsufficientFundsException(player.getUsername(), cost, player.getCash());
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

Board &GameManager::getBoard() { return *board; }

const TransactionLogger &GameManager::getLogger() const { return logger; }

TransactionLogger &GameManager::getLogger() {
  return const_cast<TransactionLogger &>(
      static_cast<const GameManager *>(this)->getLogger());
}

int GameManager::getGoSalary() const { return config.getGoSalary(); }

int GameManager::getJailPosition() const {
  if (board != nullptr) {
    return board->findJailPosition();
  }
  return 10;
}

int GameManager::getBoardSize() const {
  if (board != nullptr) {
    return board->getTileCount();
  }
  return 40;
}

void GameManager::setLastDiceTotal(int total) {
  lastDiceTotal = std::max(0, total);
}

int GameManager::getLastDiceTotal() const {
  return lastDiceTotal;
}
