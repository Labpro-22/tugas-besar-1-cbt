#include "core/GoTile.hpp"

GoTile::GoTile(const std::string &code, const std::string &name, int pos,
               int salary)
    : ActionTile(code, name, pos, "go"), salary(salary) {}

void GoTile::onLanded(Player &player, GameManager &game) {
  // Implementasi untuk player apabila mendarat di petak
}

void GoTile::onPassed(Player &player, GameManager &game) {
  // Implementasi untuk player apabila melewati petak
}

int GoTile::getSalary() const { return salary; }
