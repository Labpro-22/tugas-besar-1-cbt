#include "core/GoToJailTile.hpp"

GoToJailTile::GoToJailTile(const std::string &code, const std::string &name,
                           int pos)
    : ActionTile(code, name, pos, "go_to_jail") {}

void GoToJailTile::onLanded(Player &player, GameManager &game) {
  // Implementasi override fungsi onLanded
}
