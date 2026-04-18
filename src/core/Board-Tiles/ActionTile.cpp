#include "core/ActionTile.hpp"

ActionTile::ActionTile(const std::string &code, const std::string &name, int pos, const std::string &actionType) 
  : Tile(code, name, pos, "action"), action_type(actionType) {}

std::string ActionTile::getActionType() const { return action_type; }
