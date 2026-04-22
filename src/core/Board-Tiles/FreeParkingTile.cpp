#include "../../../include/core/Board-Tiles/FreeParkingTile.hpp"
#include "../../../include/models/GameManager/GameManager.hpp"
#include "../../../include/models/GameManager/Player.hpp"

#include <iostream>

FreeParkingTile::FreeParkingTile(const std::string &code,
                                 const std::string &name, int pos)
    : ActionTile(code, name, pos, "free_parking") {}

void FreeParkingTile::onLanded(Player &player, GameManager &game) {
  (void)game;
  std::cout << player.getUsername() << " berhenti di " << getName()
            << ". Tidak ada aksi.\n";
}
