#ifndef GO_TO_JAIL_TILE_HPP
#define GO_TO_JAIL_TILE_HPP

#include "core/ActionTile.hpp"

class GoToJailTile : public ActionTile {
public:
  // Konstruktor dan destruktor
  GoToJailTile(const std::string &code, const std::string &name, int pos);
  ~GoToJailTile() = default;

  // Handler untuk pemberhentian di petak
  void onLanded(Player &player, GameManager &game) override;
};

#endif
