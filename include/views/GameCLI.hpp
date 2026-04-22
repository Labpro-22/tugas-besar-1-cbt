#ifndef GAME_CLI_HPP
#define GAME_CLI_HPP

#include <string>
#include <vector>

#include "../models/Command.hpp"
#include "../utils/CommandParser.hpp"
#include "AuctionView.hpp"
#include "BoardRenderer.hpp"
#include "InputHandler.hpp"
#include "LogView.hpp"
#include "PropertyView.hpp"
#include "StatusView.hpp"

class Player;

class GameCLI {
private:
    InputHandler input;
    BoardRenderer boardRenderer;
    PropertyView propertyView;
    AuctionView auctionView;
    LogView logView;
    StatusView statusView;
    CommandParser commandParser;

public:
    GameCLI();

    void showMainMenu() const;
    void showTurnHeader(Player* current, int turn, int maxTurn) const;
    Command readCommand();
    void showMessage(const std::string& msg) const;
    void showError(const std::string& err) const;
    void showWinner(const std::vector<Player*>& winners) const;
    void showLoadSuccess(const std::string& filename) const;
    void showSaveSuccess(const std::string& filename) const;

    InputHandler& getInputHandler();
    BoardRenderer& getBoardRenderer();
    PropertyView& getPropertyView();
    AuctionView& getAuctionView();
    LogView& getLogView();
    StatusView& getStatusView();
    CommandParser& getCommandParser();
};

#endif
