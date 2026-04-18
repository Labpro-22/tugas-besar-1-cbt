#include "Auction.hpp"
#include <iostream>
#include <algorithm>

// Constructor
Auction::Auction(Property* prop, vector<Player*> participants) {
    this->property = prop;
    this->participants = participants; // Menyimpan referensi/pointer pemain asli
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
        if (!player->canPay(amount)) {
            cout << "Uang " << player->getUsername() << " tidak cukup untuk tawaran " << amount << "!\n";
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

    cout << "-- " << player->getUsername() << " memilih PASS (Mundur).\n";
    passCount++;
    
    if (passCount >= participants.size() - 1 && winnerIndex != -1) {
        determineWinner(); // Pemenang ditemukan
    } 
    else if (passCount >= participants.size()) {
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
        cout << "Tidak ada yang menawar. Properti tidak terjual.\n\n";
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