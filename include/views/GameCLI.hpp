#ifndef GAME_CLI_HPP
#define GAME_CLI_HPP

#include <string>
#include <vector>

#include "models/Command.hpp"
#include "models/GameManager/Player.hpp"
#include "utils/CommandParser.hpp"
#include "views/AuctionView.hpp"
#include "views/BoardRenderer.hpp"
#include "views/InputHandler.hpp"
#include "views/LogView.hpp"
#include "views/PropertyView.hpp"
#include "views/StatusView.hpp"

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