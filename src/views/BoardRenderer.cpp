#include "../../../include/views/BoardRenderer.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../../include/core/Board-Tiles/Board.hpp"
#include "../../../include/core/Board-Tiles/Tile.hpp"
#include "../../../include/models/GameManager/Player.hpp"

namespace {

std::string truncateText(const std::string& text, std::size_t maxWidth) {
    if (text.size() <= maxWidth) {
        return text;
    }

    if (maxWidth <= 3) {
        return text.substr(0, maxWidth);
    }

    return text.substr(0, maxWidth - 3) + "...";
}

std::string joinPlayerNames(const std::vector<Player*>& players, std::size_t maxWidth) {
    if (players.empty()) {
        return "-";
    }

    std::stringstream ss;
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i] == nullptr) {
            continue;
        }

        if (ss.tellp() > 0) {
            ss << ", ";
        }

        ss << players[i]->getUsername();
    }

    std::string joined = ss.str();
    if (joined.empty()) {
        return "-";
    }

    return truncateText(joined, maxWidth);
}

}  // namespace

BoardRenderer::BoardRenderer(int cellWidth, const std::string& borderStyle)
    : cellWidth(cellWidth), borderStyle(borderStyle) {
    colorCodeMap["property"] = "[PR]";
    colorCodeMap["action"] = "[AC]";
    colorCodeMap["tax"] = "[TX]";
    colorCodeMap["card"] = "[CD]";
    colorCodeMap["festival"] = "[FV]";
    colorCodeMap["base"] = "[DF]";
    colorCodeMap["DEFAULT"] = "[DF]";
}

std::string BoardRenderer::renderBoard(Board* board, const std::vector<Player*>& players, int currentTurn, int maxTurn) const {
    std::stringstream ss;

    if (board == nullptr) {
        ss << "Board tidak valid.\n";
        return ss.str();
    }

    ss << "=== PAPAN NIMONSPOLI ===\n";
    ss << "TURN " << currentTurn << " / " << maxTurn << "\n\n";

    const int tileCount = board->getTileCount();
    if (tileCount <= 0) {
        ss << "Board belum diinisialisasi.\n";
        return ss.str();
    }

    for (int i = 0; i < tileCount; ++i) {
        Tile* tile = &board->getTile(i);
        std::vector<Player*> playersOnTile;

        if (!players.empty()) {
            for (Player* player : players) {
                if (player != nullptr && player->getPosition() == i) {
                    playersOnTile.push_back(player);
                }
            }
        } else if (tile != nullptr) {
            playersOnTile = tile->getPlayerList();
        }

        std::vector<std::string> tileLines = renderTile(tile, playersOnTile);
        for (const std::string& line : tileLines) {
            ss << line << '\n';
        }

        ss << std::string(std::max(cellWidth, 52), borderStyle.empty() ? '-' : borderStyle.front()) << '\n';
    }

    ss << '\n';
    std::vector<std::string> legend = renderLegend();
    for (const std::string& line : legend) {
        ss << line << '\n';
    }

    return ss.str();
}

std::vector<std::string> BoardRenderer::renderTile(Tile* tile, const std::vector<Player*>& playersOnTile) const {
    std::vector<std::string> lines;

    if (tile == nullptr) {
        lines.push_back("[TILE INVALID]");
        return lines;
    }

    std::stringstream header;
    header << getTileColorCode(tile)
           << " Pos " << std::setw(2) << tile->getPosition()
           << " | " << std::setw(3) << tile->getCode()
           << " | " << truncateText(tile->getName(), static_cast<std::size_t>(std::max(cellWidth, 16)));

    std::stringstream typeLine;
    typeLine << "   Tipe   : " << tile->getType();

    std::stringstream playerLine;
    playerLine << "   Pemain : " << joinPlayerNames(playersOnTile, static_cast<std::size_t>(std::max(cellWidth, 24)));

    lines.push_back(header.str());
    lines.push_back(typeLine.str());
    lines.push_back(playerLine.str());

    return lines;
}

std::vector<std::string> BoardRenderer::renderLegend() const {
    std::vector<std::string> legend;

    legend.push_back("=== LEGENDA PETAK ===");
    legend.push_back("[PR] = Property");
    legend.push_back("[AC] = Action");
    legend.push_back("[TX] = Tax");
    legend.push_back("[CD] = Card");
    legend.push_back("[FV] = Festival");
    legend.push_back("[DF] = Tidak dikenali");

    return legend;
}

std::string BoardRenderer::getTileColorCode(Tile* tile) const {
    if (tile == nullptr) {
        return "[DF]";
    }

    std::string category = tile->getType();

    auto it = colorCodeMap.find(category);
    if (it != colorCodeMap.end()) {
        return it->second;
    }

    return "[DF]";
}
