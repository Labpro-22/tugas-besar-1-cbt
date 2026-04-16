#include "core/FestivalTile.hpp"

FestivalTile::FestivalTile(const std::string &code, const std::string &name,
                           int pos)
    : Tile(code, name, pos) {
  tile_type = "festival";
}

void FestivalTile::onLanded(Player &player, GameManager &game) {
  // Implementasi handler untuk festival
}
