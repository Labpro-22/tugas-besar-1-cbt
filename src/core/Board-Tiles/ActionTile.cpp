#include "../../../include/core/Board-Tiles/ActionTile.hpp"

ActionTile::ActionTile(const std::string &code, const std::string &name, int pos, const std::string &actionType) 
    : Tile(code, name, pos, "action"), action_type(actionType) {}

const ActionTile *ActionTile::asActionTile() const { return this; }

std::string ActionTile::getActionType() const { return action_type; }
