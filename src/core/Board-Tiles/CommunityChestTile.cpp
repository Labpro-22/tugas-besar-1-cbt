#include "core/Board-Tiles/CommunityChestTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <iostream>

CommunityChestTile::CommunityChestTile(const std::string &code,
                                                                                                                                                            const std::string &name, int pos)
                : CardTile(code, name, pos, "community_chest") {}

void CommunityChestTile::drawCardandExecute(Player &player, GameManager &game) {
        // Placeholder: akan diimplementasikan sepenuhnya dengan konfigurasi
        (void)player;
        (void)game;
        std::cout << "CommunityChestTile: Mengambil kartu dana umum (placeholder)"
                                                << std::endl;
}

void CommunityChestTile::onLanded(Player &player, GameManager &game) {
        drawCardandExecute(player, game);
}
