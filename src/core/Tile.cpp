#include "core/Tile.hpp"

Tile::Tile(const std::string &code, const std::string &name, int pos)
    : id(code), name(name), position(pos), landed(false), tile_type("base") {}

std::string Tile::getCode() const { return id; }

std::string Tile::getName() const { return name; }

int Tile::getPosition() const { return position; }

bool Tile::isLanded() const { return !on_tile.empty(); }

std::string Tile::getType() const { return tile_type; }

std::vector<Player *> Tile::getPlayerList() const { return on_tile; }

void Tile::addPlayer(Player &player) {
  auto target = std::find(on_tile.begin(), on_tile.end(), &player);
  if (target == on_tile.end()) {
    on_tile.push_back(&player);
  }
}

void Tile::removePlayer(Player &player) {
  auto target = std::find(on_tile.begin(), on_tile.end(), &player);
  if (target != on_tile.end()) {
    on_tile.erase(target);
  }
}
