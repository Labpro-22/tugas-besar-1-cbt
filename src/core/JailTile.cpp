#include "core/JailTile.hpp"

JailTile::JailTile(const std::string &code, const std::string &name, int pos)
    : ActionTile(code, name, pos, "jail") {}

void JailTile::onLanded(Player &player, GameManager &game) {
  // Implementasi harusnya kosong (?) soalnya ngak ngapa2in
}
