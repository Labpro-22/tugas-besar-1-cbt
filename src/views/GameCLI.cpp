#include "views/GameCLI.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "models/GameManager/Player.hpp"

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
    while (true) {
        std::string raw = input.readLine();
        if (raw.empty() && !input.isStreamGood()) {
            return Command();
        }

        Command cmd = commandParser.parse(raw);

        if (commandParser.validate(cmd)) {
            return cmd;
        }

        if (cmd.name.empty()) {
            std::cout << "Masukkan command yang valid.\n";
        } else {
            std::cout << "Command tidak dikenali atau jumlah argumen tidak sesuai.\n";
        }
    }
}

void GameCLI::showMessage(const std::string& msg) const {
    std::cout << msg << '\n';
}

void GameCLI::showError(const std::string& err) const {
    std::string fullError = "Error: " + err + "\n";
    std::cerr << fullError;
}

void GameCLI::showWinner(const std::vector<Player*>& winners) const {
    statusView.showVictory(winners);
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
