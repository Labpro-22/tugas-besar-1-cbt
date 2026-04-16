#include "core/CommunityChestTile.hpp"

CommunityChestTile::CommunityChestTile(const std::string &code,
                                       const std::string &name, int pos)
    : CardTile(code, name, pos, "community_chest") {}

void CommunityChestTile::drawCardandExecute(Player &player, GameManager &game) {
  // Implementasi untuk menghandle pengambilan kartu
}

void CommunityChestTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
