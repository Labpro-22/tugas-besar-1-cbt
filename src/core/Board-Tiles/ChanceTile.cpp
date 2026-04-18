#include "core/Board-Tiles/ChanceTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <iostream>

ChanceTile::ChanceTile(const std::string &code, const std::string &name,
                                                                                            int pos)
                : CardTile(code, name, pos, "chance") {}

void ChanceTile::drawCardandExecute(Player &player, GameManager &game) {
        // Placeholder: akan diimplementasikan sepenuhnya dengan konfigurasi
        (void)player;
        (void)game;
        std::cout << "ChanceTile: Mengambil kartu kesempatan (placeholder)"
                                                << std::endl;
}

void ChanceTile::onLanded(Player &player, GameManager &game) {
        drawCardandExecute(player, game);
}
