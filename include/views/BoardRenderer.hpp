#ifndef BOARD_RENDERER_HPP
#define BOARD_RENDERER_HPP

#include <map>
#include <string>
#include <vector>

#include "core/Board.hpp"
#include "core/Tile.hpp"
#include "models/GameManager/Player.hpp"

class BoardRenderer {
private:
    int cellWidth;
    std::map<std::string, std::string> colorCodeMap;
    std::string borderStyle;

public:
    BoardRenderer(int cellWidth = 10, const std::string& borderStyle = "-");

    std::string renderBoard(Board* board, const std::vector<Player*>& players, int currentTurn, int maxTurn) const;
    std::vector<std::string> renderTile(Tile* tile, const std::vector<Player*>& playersOnTile) const;
    std::vector<std::string> renderLegend() const;
    std::string getTileColorCode(Tile* tile) const;
};

#endif