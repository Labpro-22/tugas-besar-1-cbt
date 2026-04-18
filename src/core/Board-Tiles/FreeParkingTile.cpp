#include "core/Board-Tiles/FreeParkingTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <iostream>

FreeParkingTile::FreeParkingTile(const std::string &code,
                                                                                                                                    const std::string &name, int pos)
                : ActionTile(code, name, pos, "free_parking") {}

void FreeParkingTile::onLanded(Player &player, GameManager &game) {
        // Placeholder: Free Parking — tidak ada aksi spesial
        (void)player;
        (void)game;
}
