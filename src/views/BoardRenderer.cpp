#include "views/BoardRenderer.hpp"

#include <sstream>
#include <iostream>

BoardRenderer::BoardRenderer(int cellWidth, const std::string& borderStyle)
    : cellWidth(cellWidth), borderStyle(borderStyle) {
    colorCodeMap["COKLAT"] = "[CK]";
    colorCodeMap["BIRU_MUDA"] = "[BM]";
    colorCodeMap["MERAH_MUDA"] = "[PK]";
    colorCodeMap["ORANGE"] = "[OR]";
    colorCodeMap["MERAH"] = "[MR]";
    colorCodeMap["KUNING"] = "[KN]";
    colorCodeMap["HIJAU"] = "[HJ]";
    colorCodeMap["BIRU_TUA"] = "[BT]";
    colorCodeMap["DEFAULT"] = "[DF]";
    colorCodeMap["ABU_ABU"] = "[AB]";
}

std::string BoardRenderer::renderBoard(Board* board, const std::vector<Player*>& players, int currentTurn, int maxTurn) const {
    std::stringstream ss;

    if (board == nullptr) {
        ss << "Board tidak valid.\n";
        return ss.str();
    }

    ss << "=== PAPAN NIMONSPOLI ===\n";
    ss << "TURN " << currentTurn << " / " << maxTurn << "\n\n";

    const std::vector<Tile*>& tiles = board->getTiles();

    for (std::size_t i = 0; i < tiles.size(); ++i) {
        std::vector<Player*> playersOnTile;

        for (Player* player : players) {
            if (player != nullptr && player->getPosition() == static_cast<int>(i)) {
                playersOnTile.push_back(player);
            }
        }

        std::vector<std::string> tileLines = renderTile(tiles[i], playersOnTile);
        for (const std::string& line : tileLines) {
            ss << line << '\n';
        }

        ss << std::string(cellWidth + 6, '-') << '\n';
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

    std::stringstream line1;
    std::stringstream line2;

    line1 << getTileColorCode(tile) << " " << tile->getCode() << " - " << tile->getName();

    if (playersOnTile.empty()) {
        line2 << "(tidak ada pemain)";
    } else {
        for (std::size_t i = 0; i < playersOnTile.size(); ++i) {
            if (playersOnTile[i] != nullptr) {
                if (i > 0) {
                    line2 << " ";
                }
                line2 << playersOnTile[i]->getUsername();
            }
        }
    }

    lines.push_back(line1.str());
    lines.push_back(line2.str());

    return lines;
}

std::vector<std::string> BoardRenderer::renderLegend() const {
    std::vector<std::string> legend;

    legend.push_back("=== LEGENDA WARNA ===");
    legend.push_back("[CK] = Coklat");
    legend.push_back("[BM] = Biru Muda");
    legend.push_back("[PK] = Pink / Merah Muda");
    legend.push_back("[OR] = Orange");
    legend.push_back("[MR] = Merah");
    legend.push_back("[KN] = Kuning");
    legend.push_back("[HJ] = Hijau");
    legend.push_back("[BT] = Biru Tua");
    legend.push_back("[AB] = Abu-Abu / Utilitas");
    legend.push_back("[DF] = Default / Aksi");

    return legend;
}

std::string BoardRenderer::getTileColorCode(Tile* tile) const {
    if (tile == nullptr) {
        return "[??]";
    }

    std::string category = tile->getCategory();

    auto it = colorCodeMap.find(category);
    if (it != colorCodeMap.end()) {
        return it->second;
    }

    return "[??]";
}