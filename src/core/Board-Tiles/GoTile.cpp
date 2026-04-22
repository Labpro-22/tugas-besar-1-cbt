#include "core/Board-Tiles/GoTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <iostream>

GoTile::GoTile(const std::string &code, const std::string &name, int pos,
               int salary)
    : ActionTile(code, name, pos, "go"), salary(salary) {}

void GoTile::onLanded(Player &player, GameManager &game) {
  (void)game;
  std::cout << player.getUsername() << " berhenti di Petak Mulai.\n";
}

void GoTile::onPassed(Player &player, GameManager &game) {
  game.executeSalary(player, getSalary());
}

int GoTile::getSalary() const { return salary; }
