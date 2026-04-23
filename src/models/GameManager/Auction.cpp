#include "models/GameManager/Auction.hpp"
#include "models/Property/Property.hpp"
#include "exception/NimonspoliExceptions.hpp"
#include <iostream>
#include <algorithm>
#include <exception>


Auction::Auction(Property* prop, vector<Player*> participants) {
    this->property = prop;
    this->participants = participants;
    this->currentBid = 0;
    
    this->minimumBid = prop->getBuyPrice() / 2; 
    
    this->winnerIndex = -1;
    this->passCount = 0;
    this->currentParticipantIndex = 0;
    this->isActive = false;
}

void Auction::start() {
    if (participants.empty()) {
        cout << "Tidak ada peserta lelang.\n";
        return;
    }
    
    isActive = true;
    
    cout << "\n=== LELANG " << property->getName() << " DIMULAI ===\n";
    cout << "Harga Minimum (Open Bid): " << minimumBid << "\n";
    cout << "Giliran pertama: " << participants[currentParticipantIndex]->getUsername() << "\n";
}

bool Auction::submitBid(Player* player, int amount) {
    if (!isActive){
        return false;
    }
    if (participants[currentParticipantIndex] != player) {
        cout << "Sabar! Belum giliran " << player->getUsername() << " untuk menawar.\n";
        return false;
    }
    if (amount > currentBid && amount >= minimumBid) {
        try {
            player->ensureCanPay(amount);
        } catch (const std::exception& e) {
            cout << e.what() << "\n";
            return false;
        }

        currentBid = amount;
        bids[player->getUsername()] = amount;
        winnerIndex = currentParticipantIndex; 
        passCount = 0; 

        cout << ">> " << player->getUsername() << " menawar sebesar " << amount << "!\n";
        nextParticipant();
        return true;
    } else {
        cout << "Gagal! Tawaran harus lebih besar dari " << max(currentBid, minimumBid) << ".\n";
        return false;
    }
}

void Auction::pass(Player* player) {
    if (!isActive) return;

    if (participants[currentParticipantIndex] != player) {
        cout << "Bukan giliran " << player->getUsername() << ".\n";
        return;
    }

    if (winnerIndex == -1 && passCount == static_cast<int>(participants.size()) - 1) {
        cout << "Pemain lain sudah PASS. Sebagai penawar terakhir, " 
             << player->getUsername() << " WAJIB melakukan bid (minimal " 
             << minimumBid << ")!\n";
        return;
    }

    cout << "-- " << player->getUsername() << " memilih PASS (Mundur).\n";
    passCount++;

    if (winnerIndex != -1 && passCount >= static_cast<int>(participants.size()) - 1) {
        determineWinner();
    } 

    else if (passCount >= static_cast<int>(participants.size())) {
        determineWinner();
    } 
    else {
        nextParticipant();
    }
}

void Auction::nextParticipant() {
    currentParticipantIndex = (currentParticipantIndex + 1) % participants.size();
    
    if (isActive) {
        cout << "Giliran menawar selanjutnya: " 
             << participants[currentParticipantIndex]->getUsername() << "\n";
    }
}

void Auction::determineWinner() {
    isActive = false;
    cout << "\n=== LELANG DITUTUP ===\n";
    
    if (winnerIndex != -1) {
        cout << "Selamat! Pemenangnya adalah " << participants[winnerIndex]->getUsername() 
             << " dengan harga " << currentBid << "!\n\n";
    } else {
        cout << "Tidak ada yang menawar. Properti dijual tanpa penawar.\n\n";
    }
}

Player* Auction::getWinner() {
    if (!isActive && winnerIndex != -1) {
        return participants[winnerIndex];
    }
    return nullptr; 
}

int Auction::getWinningBid() {
    return currentBid;
}

bool Auction::isOpen() const {
    return isActive;
}

Player* Auction::getCurrentParticipant() const {
    if (!isActive || participants.empty()) {
        return nullptr;
    }

    return participants[currentParticipantIndex];
}

int Auction::getCurrentBid() const {
    return currentBid;
}

int Auction::getMinimumBid() const {
    return minimumBid;
}
