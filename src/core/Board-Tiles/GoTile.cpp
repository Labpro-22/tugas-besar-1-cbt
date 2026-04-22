#include "core/Board-Tiles/GoTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

GoTile::GoTile(const std::string &code, const std::string &name, int pos,
               int salary)
    : ActionTile(code, name, pos, "go"), salary(salary) {}

void GoTile::onLanded(Player &player, GameManager &game) {
  game.executeSalary(player, getSalary());
}

void GoTile::onPassed(Player &player, GameManager &game) {
  game.executeSalary(player, getSalary());
}

int GoTile::getSalary() const { return salary; }
