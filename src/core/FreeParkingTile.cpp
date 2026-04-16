#include "core/FreeParkingTile.hpp"

FreeParkingTile::FreeParkingTile(const std::string &code,
                                 const std::string &name, int pos)
    : ActionTile(code, name, pos, "free_parking") {}

void FreeParkingTile::onLanded(Player &player, GameManager &game) {
  // Implementasi tile free parking
}
