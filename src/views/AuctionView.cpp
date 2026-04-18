#include "views/AuctionView.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>

AuctionView::AuctionView() : highestBidShown(0), currentPropertyCode("") {}

void AuctionView::showAuctionStart(Property* property, const std::vector<Player*>& order) {
    highestBidShown = 0;

    if (property == nullptr) {
        currentPropertyCode = "";
        std::cout << "Properti lelang tidak valid.\n";
        return;
    }

    currentPropertyCode = property->getCode();

    std::cout << "Properti " << property->getName()
              << " (" << property->getCode() << ") akan dilelang!\n";

    if (!order.empty()) {
        std::cout << "Urutan lelang:\n";
        for (std::size_t i = 0; i < order.size(); ++i) {
            if (order[i] != nullptr) {
                std::cout << (i + 1) << ". " << order[i]->getUsername() << '\n';
            }
        }
    }
}

Command AuctionView::promptAuctionAction(Player* player, int currentBid) {
    if (player == nullptr) {
        std::cout << "Pemain tidak valid.\n";
        return Command();
    }

    std::cout << "Giliran: " << player->getUsername() << '\n';
    std::cout << "Penawaran tertinggi saat ini: M" << currentBid << '\n';
    std::cout << "Aksi (PASS / BID <jumlah>): ";

    std::string line;
    std::getline(std::cin >> std::ws, line);

    std::stringstream ss(line);
    std::string name;
    ss >> name;

    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });

    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg) {
        args.push_back(arg);
    }

    if (name.empty()) {
        return Command();
    }

    return Command(name, args);
}

void AuctionView::showBidUpdate(Player* bidder, int amount) {
    if (bidder == nullptr) {
        std::cout << "Bidder tidak valid.\n";
        return;
    }

    highestBidShown = amount;

    std::cout << "Penawaran tertinggi: M" << amount
              << " (" << bidder->getUsername() << ")\n";
}

void AuctionView::showAuctionWinner(Player* winner, int amount, Property* property) {
    std::cout << "Lelang selesai!\n";

    if (winner == nullptr || property == nullptr) {
        std::cout << "Data pemenang lelang tidak lengkap.\n";
        return;
    }

    std::cout << "Pemenang: " << winner->getUsername() << '\n';
    std::cout << "Harga akhir: M" << amount << '\n';
    std::cout << "Properti " << property->getName()
              << " (" << property->getCode() << ") kini dimiliki "
              << winner->getUsername() << ".\n";
}
