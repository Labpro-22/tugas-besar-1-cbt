#include "data/GameSessionPersistence.hpp"

#include "app/GameSession.hpp"
#include "app/GameSessionUtil.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "models/Card/DemolitionCard.hpp"
#include "models/Card/DiscountCard.hpp"
#include "models/Card/LassoCard.hpp"
#include "models/Card/MoveCard.hpp"
#include "models/Card/ShieldCard.hpp"
#include "models/Card/SkillCard.hpp"
#include "models/Card/TeleportCard.hpp"
#include "models/GameManager/LogEntry.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"

using namespace app;

bool GameSessionPersistence::save(const GameSession& session,
                                  const std::string& rawFilename) const {
    const Configuration& configuration = session.configuration;
    const GameManager& game = session.game;
    const GameSessionQueries& queries = session.queries;
    const std::vector<std::string>& skillDeck = session.skillDeck;
    const std::vector<std::string>& skillDiscard = session.skillDiscard;

    std::filesystem::path path(rawFilename);
    if (path.extension().empty()) {
        path.replace_extension(".txt");
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
    file << game.getCurrentTurn() << " " << game.getMaxTurn() << "\n";
    file << game.getPlayers().size() << "\n";

    for (const Player& player : game.getPlayers()) {
        const int playerPosition = player.getPosition();
        const std::string positionToken =
            playerPosition >= 0 && playerPosition < static_cast<int>(layout.size())
                ? layout[static_cast<std::size_t>(playerPosition)].code
                : std::to_string(playerPosition);
        file << player.getUsername() << " " << player.getCash() << " "
             << positionToken << " " << playerStatusLabel(player) << "\n";
        file << player.getCardCount() << "\n";

        for (Card* card : player.getHand()) {
            file << card->getType() << " " << card->getValue();
            if (card->getType() == "DiscountCard") {
                const DiscountCard* dc = static_cast<const DiscountCard*>(card);
                file << " " << dc->getRemainingDuration();
            } else {
                file << " 0";
            }
            file << "\n";
        }
    }

    for (std::size_t i = 0; i < game.getPlayers().size(); ++i) {
        if (i > 0) {
            file << " ";
        }
        file << game.getPlayers()[i].getUsername();
    }
    file << "\n";
    if (!game.getPlayers().empty()) {
        file << game.getCurrentPlayer().getUsername() << "\n";
    } else {
        file << 0 << "\n";
    }

    std::vector<Property*> properties = queries.getAllProperties();
    file << properties.size() << "\n";
    for (Property* property : properties) {
        file << property->getCode() << " " << lowercase(property->getType()) << " ";
        if (property->getOwner() != nullptr) {
            file << property->getOwner()->getUsername();
        } else {
            file << "BANK";
        }
        file << " " << property->getStatusString() << " " << property->getFMult()
             << " " << property->getFDur() << " ";
        if (property->getBuildingCount() >= static_cast<int>(BuildingLevel::HOTEL)) {
            file << "H";
        } else {
            file << property->getBuildingCount();
        }
        file << "\n";
    }

    std::vector<std::string> serializedDeck = skillDeck;
    serializedDeck.insert(serializedDeck.end(), skillDiscard.begin(), skillDiscard.end());
    file << serializedDeck.size() << "\n";
    for (const std::string& cardType : serializedDeck) {
        file << cardType << "\n";
    }

    std::vector<LogEntry> entries = game.getLogger().getEntries();
    file << entries.size() << "\n";
    for (const LogEntry& entry : entries) {
        file << entry.turn << " " << entry.username << " " << entry.actionType
             << " " << entry.detail << "\n";
    }

    return true;
}

bool GameSessionPersistence::load(GameSession& session,
                                  const std::string& rawFilename) const {
    GameManager& game = session.game;
    const GameSessionQueries& queries = session.queries;
    std::vector<std::string>& skillDeck = session.skillDeck;
    std::vector<std::string>& skillDiscard = session.skillDiscard;
    std::map<std::string, int>& jailAttemptCounts = session.jailAttemptCounts;

    std::filesystem::path path(rawFilename);
    if (path.extension().empty()) {
        path.replace_extension(".txt");
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    int currentTurn = 0;
    int maxTurn = 0;
    if (!(file >> currentTurn >> maxTurn)) {
        return false;
    }

    int playerCount = 0;
    if (!(file >> playerCount) || playerCount < 2 || playerCount > 4) {
        return false;
    }

    std::vector<Player>& players = game.getPlayers();
    players.clear();
    players.reserve(static_cast<std::size_t>(playerCount));
    jailAttemptCounts.clear();

    for (int i = 0; i < playerCount; ++i) {
        std::string username;
        int cash = 0;
        std::string positionToken;
        std::string status;
        if (!(file >> username >> cash >> positionToken >> status)) {
            return false;
        }

        int position = -1;
        const bool numericPosition =
            !positionToken.empty() &&
            std::all_of(positionToken.begin(), positionToken.end(), [](unsigned char ch) {
                return std::isdigit(ch) != 0;
            });
        if (numericPosition) {
            position = std::stoi(positionToken);
        } else {
            position = queries.findTilePositionByCode(positionToken);
        }
        if (position < 0) {
            return false;
        }

        int handCount = 0;
        if (!(file >> handCount)) {
            return false;
        }

        std::vector<Card*> hand;
        for (int c = 0; c < handCount; ++c) {
            std::string type;
            int value = 0;
            int duration = 0;
            if (!(file >> type >> value >> duration)) {
                return false;
            }

            SkillCard* card =
                session.createSkillCardInstance(type, value, duration);
            if (card != nullptr) {
                hand.push_back(card);
            }
        }

        PlayerStatus playerStatus = ACTIVE;
        const std::string normalizedStatus = uppercase(status);
        if (normalizedStatus == "BANKRUPT") {
            playerStatus = BANKRUPT;
        } else if (normalizedStatus == "JAILED") {
            playerStatus = JAILED;
        }

        players.emplace_back(username, cash, playerStatus, position, hand);
    }

    std::string turnOrderLine;
    std::getline(file >> std::ws, turnOrderLine);

    std::string activePlayerToken;
    if (!(file >> activePlayerToken)) {
        return false;
    }
    int activePlayerIndex = 0;
    const bool numericActivePlayer =
        !activePlayerToken.empty() &&
        std::all_of(activePlayerToken.begin(), activePlayerToken.end(),
                    [](unsigned char ch) { return std::isdigit(ch) != 0; });
    if (numericActivePlayer) {
        activePlayerIndex = std::stoi(activePlayerToken);
    } else {
        auto activeIt = std::find_if(players.begin(), players.end(),
                                     [&](const Player& player) {
                                         return player.getUsername() == activePlayerToken;
                                     });
        if (activeIt == players.end()) {
            return false;
        }
        activePlayerIndex = static_cast<int>(std::distance(players.begin(), activeIt));
    }

    int propertyCount = 0;
    if (!(file >> propertyCount)) {
        return false;
    }

    for (int i = 0; i < propertyCount; ++i) {
        std::string code;
        std::string type;
        std::string ownerName;
        std::string status;
        int festivalMultiplier = 1;
        int festivalDuration = 0;
        std::string buildingState;

        if (!(file >> code >> type >> ownerName >> status >> festivalMultiplier >>
              festivalDuration >> buildingState)) {
            return false;
        }

        Property* property = queries.findPropertyByCode(code);
        if (property == nullptr) {
            continue;
        }

        property->setStatusStr(status);
        property->setFestival(festivalMultiplier, festivalDuration);
        if (buildingState == "H") {
            property->setBuildingCount(static_cast<int>(BuildingLevel::HOTEL));
        } else {
            property->setBuildingCount(std::stoi(buildingState));
        }

        if (uppercase(ownerName) == "BANK") {
            property->setOwner(nullptr);
            continue;
        }

        auto playerIt = std::find_if(
            players.begin(), players.end(), [&](const Player& player) {
                return player.getUsername() == ownerName;
            });
        if (playerIt != players.end()) {
            property->setOwner(&(*playerIt));
            playerIt->addProperty(property);
        }
    }

    int deckCount = 0;
    if (!(file >> deckCount)) {
        return false;
    }
    skillDeck.clear();
    skillDiscard.clear();
    std::string discardLine;
    std::getline(file, discardLine);
    for (int i = 0; i < deckCount; ++i) {
        std::getline(file, discardLine);
        discardLine = trim(discardLine);
        if (!discardLine.empty()) {
            skillDeck.push_back(discardLine);
        }
    }

    int logCount = 0;
    if (!(file >> logCount)) {
        return false;
    }

    game.getLogger().clear();
    std::getline(file, discardLine);
    for (int i = 0; i < logCount; ++i) {
        std::string line;
        std::getline(file, line);
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 3) {
            continue;
        }

        const int turn = std::stoi(tokens[0]);
        const std::string username = tokens[1];
        const std::string action = tokens[2];
        std::string detail;
        if (tokens.size() > 3) {
            const std::size_t detailPos = line.find(action);
            if (detailPos != std::string::npos) {
                detail = trim(line.substr(detailPos + action.size()));
            }
        }
        game.getLogger().log(turn, username, action, detail);
    }

    game.setCurrentTurn(currentTurn);
    game.setMaxTurn(maxTurn);
    game.setActivePlayerIndex(activePlayerIndex);
    for (Player& player : players) {
        if (player.getStatus() == JAILED) {
            jailAttemptCounts[player.getUsername()] = 0;
        }
    }
    session.diceRolledThisTurn = false;
    return true;
}

bool GameSession::saveToFile(const std::string& filename) const {
    return persistence.save(*this, filename);
}

bool GameSession::loadFromFile(const std::string& filename) {
    return persistence.load(*this, filename);
}
