#ifndef FESTIVAL_TILE_HPP
#define FESTIVAL_TILE_HPP

#include "core/Tile.hpp"

class FestivalTile : public Tile {
public:
  // Konstruktor dan destruktor
  FestivalTile(const std::string &code, const std::string &name, int pos);
  ~FestivalTile() = default;

  // Menghandle kasus apabila ada player di petak
  void onLanded(Player &player, GameManager &game) override;
};

#endif
