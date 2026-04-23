#ifndef APP_GAME_SESSION_HPP
#define APP_GAME_SESSION_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../core/Board-Tiles/Board.hpp"
#include "GameSessionQueries.hpp"
#include "models/Card/SkillCardFactory.hpp"
#include "../models/Card/SkillCard.hpp"
#include "../models/Card/CardDeck.hpp"
#include "../models/GameManager/Dice.hpp"
#include "../models/GameManager/GameManager.hpp"
#include "../data/Configuration.hpp"
#include "../data/GameSessionPersistence.hpp"
#include "../views/BoardRenderer.hpp"
#include "../views/GameCLI.hpp"

class PlayerSnapshot {
public:
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

class TileSnapshot {
public:
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
    int buyPrice = 0;
    int rentPrice = 0;
    int buildingPrice = 0;
    int mortgagePrice = 0;
};

class LogSnapshot {
public:
    int turn = 0;
    std::string username;
    std::string actionType;
    std::string detail;
};

class GameSnapshot {
public:
    bool gameStarted = false;
    bool gameOver = false;
    bool hasWinnerSummary = false;
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
    std::string gameOverReason;
    std::vector<std::string> winnerNames;
    int winnerCash = 0;
    int winnerPropertyCount = 0;
    int winnerCardCount = 0;
    std::vector<PlayerSnapshot> players;
    std::vector<TileSnapshot> tiles;
    std::vector<LogSnapshot> logs;
};

class Property;
class Street;

class GameSession {
public:
    using SnapshotCallback = std::function<void(const GameSnapshot&)>;
    friend class GameSessionPersistence;

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
    GameSessionQueries queries;
    GameSessionPersistence persistence;
    bool running;
    bool gameStarted;
    bool turnActionTaken;
    bool diceRolledThisTurn;
    bool gameOverAnnounced;
    std::string gameOverReason;
    std::vector<std::string> winnerNames;
    int winnerCash;
    int winnerPropertyCount;
    int winnerCardCount;
    std::string startupMode;
    std::string startupPrompt;
    int startupExpectedPlayers;
    int startupCollectedPlayers;
    SnapshotCallback snapshotCallback;
    std::map<std::string, int> jailAttemptCounts;
    SkillCardFactory skillCardFactory;
    CardDeck<std::string> skillDeck;

    bool initializeGameFromMenu();
    bool initializeNewGame();
    bool initializeLoadedGame(const std::string& filename);
    void initializePlayers(const std::vector<std::string>& usernames);
    void prepareFreshBoard();
    void resetSessionState();
    void printWelcome() const;
    void printStartupMenu() const;
    void startTurn(bool drawSkillCard = true);
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
    void markPlayerJailed(Player& player);
    void releasePlayerFromJail(Player& player);
    int getJailAttemptCount(const Player& player) const;
    void initializeSkillDeck();
    void shuffleSkillDeck();
    void ensureSkillDeckAvailable();
    SkillCard* createSkillCardInstance(const std::string& type,
                                       int value = 0,
                                       int duration = 0);
    SkillCard* drawSkillCard();
    void awardSkillCardAtTurnStart();
    void discardSkillCard(Player& player, SkillCard* card);
    bool executeSkillCard(Player& player, SkillCard* card);
    void updateStartupState(const std::string& mode, const std::string& prompt,
                            int expectedPlayers = 0, int collectedPlayers = 0);
    void notifySnapshot();
    void notifySnapshotImmediate();
    GameSnapshot buildSnapshot() const;
    std::string buildHeaderText() const;
    std::string buildStatusText() const;
    std::string buildPlayerDetailText(const Player& player) const;
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};

#endif
