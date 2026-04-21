#include "core/Board-Tiles/GoToJailTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

GoToJailTile::GoToJailTile(const std::string &code, const std::string &name,
                           int pos)
    : ActionTile(code, name, pos, "go_to_jail") {}

void GoToJailTile::onLanded(Player &player, GameManager &game) {
  game.goToJail(player);
}
