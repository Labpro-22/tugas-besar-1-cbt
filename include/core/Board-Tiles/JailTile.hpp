#ifndef JAIL_TILE_HPP
#define JAIL_TILE_HPP

#include "ActionTile.hpp"

class JailTile : public ActionTile {
    public:
        // Konstruktor dan Destruktor
        JailTile(const std::string &code, const std::string &name, int pos);
        ~JailTile() = default;

        // Handler untuk pemberhentian player di petak
        void onLanded(Player &player, GameManager &game) override;
    };

#endif