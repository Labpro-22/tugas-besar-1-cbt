#include "views/GameCLI.hpp"

#include <iostream>

GameCLI::GameCLI()
    : input(),
      boardRenderer(),
      propertyView(),
      auctionView(),
      logView(),
      statusView(),
      commandParser() {}

void GameCLI::showMainMenu() const {
    std::cout << "=========================\n";
    std::cout << "       NIMONSPOLI\n";
    std::cout << "=========================\n";
    std::cout << "1. New Game\n";
    std::cout << "2. Load Game\n";
    std::cout << "0. Exit\n";
    std::cout << "=========================\n";
}

void GameCLI::showTurnHeader(Player* current, int turn, int maxTurn) const {
    std::cout << "\n=========================\n";
    std::cout << "TURN " << turn << " / " << maxTurn << '\n';

    if (current != nullptr) {
        std::cout << "Pemain aktif: " << current->getUsername() << '\n';
    } else {
        std::cout << "Pemain aktif: [TIDAK DIKETAHUI]\n";
    }

    std::cout << "=========================\n";
}

Command GameCLI::readCommand() {
    std::string raw = input.readLine();
    return commandParser.parse(raw);
}

void GameCLI::showMessage(const std::string& msg) const {
    std::cout << msg << '\n';
}

void GameCLI::showError(const std::string& err) const {
    std::cerr << "Error: " << err << '\n';
}

void GameCLI::showWinner(const std::vector<Player*>& winners) const {
    std::cout << "\n=== HASIL PERMAINAN ===\n";

    if (winners.empty()) {
        std::cout << "Tidak ada pemenang.\n";
        return;
    }

    std::cout << "Pemenang:\n";
    for (Player* player : winners) {
        if (player != nullptr) {
            std::cout << "- " << player->getUsername() << '\n';
        }
    }
}

void GameCLI::showLoadSuccess(const std::string& filename) const {
    std::cout << "Permainan berhasil dimuat dari: " << filename << '\n';
}

void GameCLI::showSaveSuccess(const std::string& filename) const {
    std::cout << "Permainan berhasil disimpan ke: " << filename << '\n';
}

InputHandler& GameCLI::getInputHandler() {
    return input;
}

BoardRenderer& GameCLI::getBoardRenderer() {
    return boardRenderer;
}

PropertyView& GameCLI::getPropertyView() {
    return propertyView;
}

AuctionView& GameCLI::getAuctionView() {
    return auctionView;
}

LogView& GameCLI::getLogView() {
    return logView;
}

StatusView& GameCLI::getStatusView() {
    return statusView;
}

CommandParser& GameCLI::getCommandParser() {
    return commandParser;
}