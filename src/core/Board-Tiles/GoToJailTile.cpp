#include "../../../include/core/Board-Tiles/GoToJailTile.hpp"
#include "../../../include/models/GameManager/GameManager.hpp"
#include "../../../include/models/GameManager/Player.hpp"

GoToJailTile::GoToJailTile(const std::string &code, const std::string &name,
                           int pos)
    : ActionTile(code, name, pos, "go_to_jail") {}

void GoToJailTile::onLanded(Player &player, GameManager &game) {
  game.goToJail(player);
}
