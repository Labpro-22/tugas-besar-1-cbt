#include "core/Board-Tiles/JailTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

JailTile::JailTile(const std::string &code, const std::string &name, int pos)
    : ActionTile(code, name, pos, "jail") {}

void JailTile::onLanded(Player &player, GameManager &game) {
  logTileEvent(game, player, "PETAK",
               player.getUsername() + " mampir di Penjara (hanya berkunjung).");
}
