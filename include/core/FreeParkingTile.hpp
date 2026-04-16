#ifndef FREE_PARKING_TILE_HPP
#define FREE_PARKING_TILE_HPP

#include "core/ActionTile.hpp"

class FreeParkingTile : public ActionTile {
public:
  FreeParkingTile(const std::string &code, const std::string &name, int pos);
  ~FreeParkingTile() = default;

  void onLanded(Player &player, GameManager &game) override;
};

#endif
