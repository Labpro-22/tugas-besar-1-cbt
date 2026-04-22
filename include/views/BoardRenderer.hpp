#ifndef BOARD_RENDERER_HPP
#define BOARD_RENDERER_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class Board;
class Tile;
class Player;

class BoardRenderer {
private:
    int cellWidth;
    std::map<std::string, std::string> colorCodeMap;
    std::string borderStyle;

    static std::string truncateText(const std::string& text,
                                    std::size_t maxWidth);
    static std::string joinPlayerNames(const std::vector<Player*>& players,
                                       std::size_t maxWidth);

public:
    BoardRenderer(int cellWidth = 10, const std::string& borderStyle = "-");

    std::string renderBoard(Board* board, const std::vector<Player*>& players, int currentTurn, int maxTurn) const;
    std::vector<std::string> renderTile(Tile* tile, const std::vector<Player*>& playersOnTile) const;
    std::vector<std::string> renderLegend() const;
    std::string getTileColorCode(Tile* tile) const;
};

#endif
