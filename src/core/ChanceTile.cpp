#include "core/ChanceTile.hpp"

ChanceTile::ChanceTile(const std::string &code, const std::string &name,
                       int pos)
    : CardTile(code, name, pos, "chance") {}

void ChanceTile::drawCardandExecute(Player &player, GameManager &game) {
  // Implementasi untuk menghandle pengambilan kartu
}

void ChanceTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
