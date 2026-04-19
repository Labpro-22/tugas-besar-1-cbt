#ifndef APP_GAME_SESSION_HPP
#define APP_GAME_SESSION_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "../core/Board-Tiles/Board.hpp"
#include "../models/GameManager/Dice.hpp"
#include "../models/GameManager/GameManager.hpp"
#include "../placeholder/Configuration.hpp"
#include "../views/BoardRenderer.hpp"
#include "../views/GameCLI.hpp"

struct PlayerSnapshot {
    std::string name;
    int cash = 0;
    int position = 0;
    int propertyCount = 0;
    int cardCount = 0;
    bool active = false;
    bool bankrupt = false;
    bool jailed = false;
    std::string detailText;
};

struct TileSnapshot {
    int position = 0;
    std::string code;
    std::string name;
    std::string type;
    std::string colorKey;
    std::string owner;
    int buildingCount = 0;
    bool mortgaged = false;
    int festivalMultiplier = 1;
    int festivalDuration = 0;
};

struct GameSnapshot {
    bool gameStarted = false;
    int currentTurn = 0;
    int maxTurn = 0;
    int activePlayerIndex = 0;
    std::string startupMode;
    std::string startupPrompt;
    int startupExpectedPlayers = 0;
    int startupCollectedPlayers = 0;
    int die1 = 0;
    int die2 = 0;
    bool hasDiceResult = false;
    std::string headerText;
    std::string statusText;
    std::vector<PlayerSnapshot> players;
    std::vector<TileSnapshot> tiles;
};

class Property;
class Street;

class GameSession {
public:
    using SnapshotCallback = std::function<void(const GameSnapshot&)>;

    GameSession();

    void setSnapshotCallback(SnapshotCallback callback);
    void requestStop();
    void run();

private:
    GameCLI cli;
    GameManager game;
    Board board;
    BoardRenderer boardRenderer;
    Dice dice;
    Configuration configuration;
    bool running;
    bool gameStarted;
    bool turnActionTaken;
    std::string startupMode;
    std::string startupPrompt;
    int startupExpectedPlayers;
    int startupCollectedPlayers;
    SnapshotCallback snapshotCallback;

    bool initializeGameFromMenu();
    bool initializeNewGame();
    bool initializeLoadedGame(const std::string& filename);
    void initializePlayers(const std::vector<std::string>& usernames);
    void prepareFreshBoard();
    void resetSessionState();
    void printWelcome() const;
    void printStartupMenu() const;
    void handleCommand(const Command& command);
    void handlePrintBoard();
    void handleRollDice(bool manual, int d1 = 0, int d2 = 0);
    void handlePrintDeed();
    void handlePrintProperties();
    void handlePrintLogs(const Command& command);
    void handleMortgage();
    void handleRedeem();
    void handleBuild();
    void handleUseAbility();
    void handleSave(const Command& command);
    void handleLoad(const Command& command) const;
    void finishTurn();
    void announceWinner();
    void updateStartupState(const std::string& mode, const std::string& prompt,
                            int expectedPlayers = 0, int collectedPlayers = 0);
    void notifySnapshot();
    void notifySnapshotImmediate();
    GameSnapshot buildSnapshot() const;
    std::string buildHeaderText() const;
    std::string buildStatusText() const;
    std::string buildPlayerDetailText(const Player& player) const;
    std::vector<Property*> getAllProperties() const;
    Property* findPropertyByCode(const std::string& code) const;
    int findTilePositionByCode(const std::string& code) const;
    std::vector<Property*> getMortgageableProperties() const;
    std::vector<Property*> getRedeemableProperties() const;
    std::vector<Street*> getBuildableStreets() const;
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};

#endif
