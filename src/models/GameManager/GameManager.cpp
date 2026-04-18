#include "GameManager.hpp"
#include "BankruptcyHandler.hpp"
#include "../Card/ActionCard.hpp"
#include <iostream>
#include <string>

using namespace std;

void GameManager::startNewGame() {
    currentTurn = 1;
    activePlayerIndex = 0;
    addLogEntry("Game Dimulai");
}

void GameManager::processTurn() {
    if (players.empty()) {
        return;
    }

    Player& currentPlayer = getCurrentPlayer();
    
    if (jailManager.isInJail(currentPlayer)) {
        jailManager.incrementJailTurn();
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
        return true;
    }

    int activeCount = 0;
    for (Player& p : players) {
        if (p.getStatus() == ACTIVE) {
            activeCount++;
        }
    }
    return (activeCount <= 1) || (maxTurn > 0 && currentTurn > maxTurn);
}

Player& GameManager::getCurrentPlayer() {
    return players[activePlayerIndex];
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
    } while (players[activePlayerIndex].getStatus() == BANKRUPT && !players.empty());
}

bool GameManager::isGameOver() {
    return checkWinCondition();
}

Player& GameManager::getWinner() {
    int maxWealth = -1;
    int winnerIndex = 0;
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i].getStatus() == ACTIVE) {
            int wealth = players[i].getTotalWealth();
            if (wealth > maxWealth) {
                maxWealth = wealth;
                winnerIndex = i;
            }
        }
    }
    return players[winnerIndex];
}

void GameManager::moveCurrentPlayer(int steps) {
    Player& player = getCurrentPlayer();
    int oldPos = player.getPosition();
    int newPos = (oldPos + steps) % 40;
    if (newPos < 0) {
        newPos += 40;
    }
    
    player.setPosition(newPos);

    if (steps > 0 && (newPos < oldPos || newPos == 0)) {
        player.addCash(200);
    }
    
    addLogEntry("Bergerak ke petak " + to_string(newPos));
}

void GameManager::movePlayerTo(Player& player, int position) {
    int oldPos = player.getPosition();
    int newPos = position % 40;
    if (newPos < 0) {
        newPos += 40;
    }

    player.setPosition(newPos);
    if (newPos < oldPos) {
        player.addCash(200);
    }
}

void GameManager::sendPlayerToJail(Player& player) {
    jailManager.sendToJail(player);
    addLogEntry(player.getUsername() + " masuk penjara");
}

int GameManager::getNearestStationPosition(int position) const {
    static const int stations[] = {5, 15, 25, 35};
    int normalizedPosition = position % 40;
    if (normalizedPosition < 0) {
        normalizedPosition += 40;
    }

    for (int station : stations) {
        if (station > normalizedPosition) {
            return station;
        }
    }

    return stations[0];
}

bool GameManager::executePurchase(Player& player, Property& prop) {
    if (prop.getOwner() != nullptr) {
        return false;
    }

    int price = prop.getBuyPrice();
    if (!player.canPay(price)) {
        return false;
    }

    player.reduceCash(price);
    player.addProperty(&prop);
    prop.setOwner(&player);
    addLogEntry("Membeli " + prop.getName());
    return true;
}

void GameManager::executeRentPayer(Player& payer, Player& owner, Property& prop, int amount) {
    if (payer.hasShieldActive()) {
        addLogEntry(payer.getUsername() + " terlindungi shield dari pembayaran sewa");
        return;
    }

    if (payer.canPay(amount)) {
        payer.reduceCash(amount);
        owner.addCash(amount);
        addLogEntry("Membayar sewa ke " + owner.getUsername());
    } else {
        BankruptcyHandler bh(payer, &owner, amount);
        if (bh.initiateLiquidation()) {
            addLogEntry("Sewa dibayar setelah likuidasi");
        } else {
            executeBankruptcy(payer, &owner, amount);
        }
    }
}

void GameManager::executeAuction(Property& prop) {
    addLogEntry("Lelang untuk " + prop.getName() + " dimulai");
}

void GameManager::executeBankruptcy(Player& debtor, Player* creditor, int amount) {
    BankruptcyHandler bh(debtor, creditor, amount);
    bh.declareBankrupt();
    if (creditor != nullptr) {
        addLogEntry("Bangkrut dan aset disita oleh " + creditor->getUsername());
    } else {
        addLogEntry("Bangkrut dan aset disita oleh bank");
    }
}

void GameManager::executeFestival(Player& player, string propCode) {
    addLogEntry("Mengadakan festival di " + propCode);
}

void GameManager::executeTaxPayment(Player& player, int amount, bool toBank) {
    if (player.hasShieldActive()) {
        addLogEntry(player.getUsername() + " terlindungi shield dari pembayaran penalti");
        return;
    }

    if (player.canPay(amount)) {
        player.reduceCash(amount);
        addLogEntry("Membayar pajak " + to_string(amount));
    } else {
        BankruptcyHandler bh(player, nullptr, amount);
        if (bh.initiateLiquidation()) {
            addLogEntry("Pajak dibayar setelah likuidasi");
        } else {
            bh.declareBankrupt();
            addLogEntry("Bangkrut karena pajak");
        }
    }
}

void GameManager::executeChanceCard(Player& player, Card& card) {
    ActionCard* actionCard = dynamic_cast<ActionCard*>(&card);
    if (actionCard == nullptr) {
        return;
    }

    actionCard->execute(&player, this);
    addLogEntry("Kartu chance diproses: " + card.getDescription());
}

void GameManager::executeCommunityChestCard(Player& player, Card& card) {
    ActionCard* actionCard = dynamic_cast<ActionCard*>(&card);
    if (actionCard == nullptr) {
        return;
    }

    actionCard->execute(&player, this);
    addLogEntry("Kartu community chest diproses: " + card.getDescription());
}

void GameManager::addLogEntry(string action) {
    string username = players.empty() ? string() : getCurrentPlayer().getUsername();
    logger.log(currentTurn, username, action, "");
}