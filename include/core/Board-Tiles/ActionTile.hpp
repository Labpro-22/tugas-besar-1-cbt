#ifndef ACTION_TILE_HPP
#define ACTION_TILE_HPP

#include "Tile.hpp"

class ActionTile : public Tile {
    protected:
        // Atribut
        std::string action_type;

    public:
        // Konstruktor dan destruktor
        ActionTile(const std::string &code, const std::string &name, int pos, const std::string &actionType);
        ~ActionTile() = default;

        // Getter
        std::string getActionType() const;
};

#endif
