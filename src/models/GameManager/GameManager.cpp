#include "GameManager.hpp"
#include "BankruptcyHandler.hpp"
#include <iostream>
#include <string>

using namespace std;

void GameManager::startNewGame() {
    currentTurn = 1;
    activePlayerIndex = 0;
    addLogEntry("Game Dimulai");
}

void GameManager::processTurn() {
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
    do {
        activePlayerIndex = (activePlayerIndex + 1) % players.size();
        if (activePlayerIndex == 0) {
            currentTurn++;
        }
    } while (players[activePlayerIndex].getStatus() == BANKRUPT);
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
    
    player.setPosition(newPos);
    
    if (newPos < oldPos || newPos == 0) {
        player.addCash(200);
    }
    
    addLogEntry("Bergerak ke petak " + to_string(newPos));
}

void GameManager::executePurchase(Player& player, Property& prop) {
    int price = prop.getBuyPrice(); 
    if (player.canPay(price)) {
        player.reduceCash(price);
        player.addProperty(&prop);
        prop.setOwner(&player);
        addLogEntry("Membeli " + prop.getName());
    }
}

void GameManager::executeRentPayer(Player& payer, Player& owner, Property& prop, int amount) {
    if (payer.canPay(amount)) {
        payer.reduceCash(amount);
        owner.addCash(amount);
        addLogEntry("Membayar sewa ke " + owner.getUsername());
    } else {
        BankruptcyHandler bh(payer, &owner, amount);
        if (bh.initiateLiquidation()) {
            payer.reduceCash(amount);
            owner.addCash(amount);
            addLogEntry("Sewa dibayar setelah likuidasi");
        } else {
            executeBankruptcy(payer, owner, amount);
        }
    }
}

void GameManager::executeAuction(Property& prop) {
    addLogEntry("Lelang untuk " + prop.getName() + " dimulai");
}

void GameManager::executeBankruptcy(Player& debtor, Player& creditor, int amount) {
    BankruptcyHandler bh(debtor, &creditor, amount);
    bh.declareBankrupt();
    bh.transferAssets();
    addLogEntry("Bangkrut dan aset disita oleh " + creditor.getUsername());
}

void GameManager::executeFestival(Player& player, string propCode) {
    addLogEntry("Mengadakan festival di " + propCode);
}

void GameManager::executeTaxPayment(Player& player, int amount, bool toBank) {
    if (player.canPay(amount)) {
        player.reduceCash(amount);
        addLogEntry("Membayar pajak " + to_string(amount));
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

void GameManager::addLogEntry(string action) {
    string username = getCurrentPlayer().getUsername();
    logger.log(currentTurn, username, action, "");
}