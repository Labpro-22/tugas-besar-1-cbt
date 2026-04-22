#ifndef APP_GAME_SESSION_QUERIES_HPP
#define APP_GAME_SESSION_QUERIES_HPP

#include <string>
#include <vector>

class Board;
class Configuration;
class GameManager;
class Property;
class Street;

class GameSessionQueries {
public:
    GameSessionQueries(Board& board, const Configuration& configuration,
                       const GameManager& game);

    std::vector<Property*> getAllProperties() const;
    Property* findPropertyByCode(const std::string& code) const;
    int findTilePositionByCode(const std::string& code) const;
    std::vector<Property*> getMortgageableProperties() const;
    std::vector<Property*> getRedeemableProperties() const;
    std::vector<Street*> getBuildableStreets() const;

private:
    Board& board;
    const Configuration& configuration;
    const GameManager& game;
};

#endif
