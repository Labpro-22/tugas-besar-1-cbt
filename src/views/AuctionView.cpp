#include "views/AuctionView.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "exception/NimonspoliExceptions.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "utils/CommandParser.hpp"
#include "views/InputHandler.hpp"


AuctionView::AuctionView() : highestBidShown(0), currentPropertyCode("") {}

void AuctionView::showAuctionStart(Property *property,
                                   const std::vector<Player *> &order) {
  highestBidShown = 0;

  if (property == nullptr) {
    currentPropertyCode = "";
    throw AuctionFailedException("Properti lelang tidak valid.");
  }

  currentPropertyCode = property->getCode();

  std::cout << "Properti " << property->getName() << " (" << property->getCode()
            << ") akan dilelang!\n";

  if (!order.empty()) {
    std::cout << "Urutan lelang:\n";
    for (std::size_t i = 0; i < order.size(); ++i) {
      if (order[i] != nullptr) {
        std::cout << (i + 1) << ". " << order[i]->getUsername() << '\n';
      }
    }
  }
}

Command AuctionView::promptAuctionAction(Player *player, int currentBid) {
  InputHandler input;
  return promptAuctionAction(player, currentBid, input);
}

Command AuctionView::promptAuctionAction(Player *player, int currentBid,
                                         InputHandler &input) {
  if (player == nullptr) {
    throw AuctionFailedException("Pemain tidak valid.");
  }

  CommandParser parser;

  while (true) {
    std::cout << "Giliran: " << player->getUsername() << '\n';
    std::cout << "Penawaran tertinggi saat ini: M" << currentBid << '\n';
    std::string line =
        input.readPromptLine("Aksi (PASS / BID <jumlah>): ", "Aksi Lelang");

    Command cmd = parser.parse(line);
    if (parser.validate(cmd) && (cmd.name == "PASS" || cmd.name == "BID")) {
      return cmd;
    }

    std::cout << "Input lelang tidak valid. Gunakan PASS atau BID <jumlah>.\n";
  }
}

void AuctionView::showBidUpdate(Player *bidder, int amount) {
  if (bidder == nullptr) {
    throw AuctionFailedException("Bidder tidak valid.");
  }

  highestBidShown = amount;

  std::cout << "Penawaran tertinggi: M" << amount << " ("
            << bidder->getUsername() << ")\n";
}

void AuctionView::showAuctionWinner(Player *winner, int amount,
                                    Property *property) {
  std::cout << "Lelang selesai!\n";

  if (winner == nullptr || property == nullptr) {
    throw AuctionFailedException("Data pemenang lelang tidak lengkap.");
  }

  std::cout << "Pemenang: " << winner->getUsername() << '\n';
  std::cout << "Harga akhir: M" << amount << '\n';
  std::cout << "Properti " << property->getName() << " (" << property->getCode()
            << ") kini dimiliki " << winner->getUsername() << ".\n";
}
