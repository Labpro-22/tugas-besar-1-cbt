#include "core/ActionTile.hpp"

ActionTile::ActionTile(const std::string &code, const std::string &name,
                       int pos, const std::string &actionType)
    : Tile(code, name, pos), action_type(actionType) {
  tile_type = "action";
}

std::string ActionTile::getActionType() const { return action_type; }
