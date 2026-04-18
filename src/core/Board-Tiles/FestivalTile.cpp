#include "core/Board-Tiles/FestivalTile.hpp"

#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>

FestivalTile::FestivalTile(const std::string &code, const std::string &name, int pos)
  : Tile(code, name, pos, "festival") {}

void FestivalTile::onLanded(Player &player, GameManager &game) {
  static std::unordered_map<PropertyTile *, int> festivalMultiplierByTile;

  Board &board = game.getBoard();
  std::vector<PropertyTile *> ownedTiles;
  ownedTiles.reserve(static_cast<std::size_t>(board.getTileCount()));

  for (int i = 0; i < board.getTileCount(); ++i) {
    Tile &tile = board.getTile(i);
    PropertyTile *propertyTile = dynamic_cast<PropertyTile *>(&tile);
    if (propertyTile == nullptr) {
      continue;
    }

    Property &property = propertyTile->getProperty();
    if (property.getOwner() == &player) {
      ownedTiles.push_back(propertyTile);
    }
  }

  if (ownedTiles.empty()) {
    return;
  }

  int selectedIndex = 0;
  std::cin >> selectedIndex;

  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(ownedTiles.size())) {
    selectedIndex = 0;
  }

  PropertyTile *selectedTile = ownedTiles[static_cast<std::size_t>(selectedIndex)];

  int currentMultiplier = 1;
  const auto multiplierIt = festivalMultiplierByTile.find(selectedTile);
  if (multiplierIt != festivalMultiplierByTile.end()) {
    currentMultiplier = multiplierIt->second;
  }

  int nextMultiplier = currentMultiplier;
  if (nextMultiplier < 8) {
    nextMultiplier *= 2;
  }

  festivalMultiplierByTile[selectedTile] = nextMultiplier;
  selectedTile->applyFestivalEffect(nextMultiplier, 3);
}
