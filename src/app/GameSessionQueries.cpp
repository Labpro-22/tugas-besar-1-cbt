#include "app/GameSessionQueries.hpp"
#include "app/GameSessionUtil.hpp"

#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "data/Configuration.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "models/Property/PropertyStatus.hpp"
#include "models/Property/Street.hpp"

#include <algorithm>

using namespace app;

GameSessionQueries::GameSessionQueries(Board& boardRef,
                                       const Configuration& configurationRef,
                                       const GameManager& gameRef)
    : board(boardRef), configuration(configurationRef), game(gameRef) {}

std::vector<Property*> GameSessionQueries::getAllProperties() const {
    std::vector<Property*> properties;
    for (int i = 0; i < board.getTileCount(); ++i) {
        Tile& tile = board.getTile(i);
        if (tile.getType() == "property") {
            PropertyTile& propTile = static_cast<PropertyTile&>(tile);
            properties.push_back(&propTile.getProperty());
        }
    }
    return properties;
}

Property* GameSessionQueries::findPropertyByCode(const std::string& code) const {
    const std::string normalizedCode = uppercase(code);
    for (Property* property : getAllProperties()) {
        if (property != nullptr &&
            uppercase(property->getCode()) == normalizedCode) {
            return property;
        }
    }
    return nullptr;
}

int GameSessionQueries::findTilePositionByCode(const std::string& code) const {
    const std::string normalized = uppercase(code);
    const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
    for (int i = 0; i < board.getTileCount(); ++i) {
        const std::string tileCode =
            i < static_cast<int>(layout.size())
                ? layout[static_cast<std::size_t>(i)].code
                : board.getTile(i).getCode();
        if (uppercase(tileCode) == normalized) {
            return i;
        }
    }
    return -1;
}

std::vector<Property*> GameSessionQueries::getMortgageableProperties() const {
    std::vector<Property*> result;
    const Player& currentPlayer = game.getCurrentPlayer();
    for (Property* property : currentPlayer.getProperties()) {
        if (property == nullptr) {
            continue;
        }
        if (property->canBeMortgaged()) {
            result.push_back(property);
        }
    }
    return result;
}

std::vector<Property*> GameSessionQueries::getRedeemableProperties() const {
    std::vector<Property*> result;
    const Player& currentPlayer = game.getCurrentPlayer();
    for (Property* property : currentPlayer.getProperties()) {
        if (property != nullptr &&
            property->getStatus() == PropertyStatus::MORTGAGED) {
            result.push_back(property);
        }
    }
    return result;
}

std::vector<Street*> GameSessionQueries::getBuildableStreets() const {
    std::vector<Street*> result;
    const Player& currentPlayer = game.getCurrentPlayer();
    for (Property* property : currentPlayer.getProperties()) {
        if (property == nullptr || property->getType() != "Street") {
            continue;
        }
        Street* street = static_cast<Street*>(property);
        if (street->getStatus() != PropertyStatus::OWNED) {
            continue;
        }
        if (!game.ownsFullColorGroup(currentPlayer, street->getColorGroup())) {
            continue;
        }
        result.push_back(street);
    }
    return result;
}
