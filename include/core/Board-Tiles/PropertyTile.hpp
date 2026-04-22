#ifndef PROPERTY_TILE_HPP
#define PROPERTY_TILE_HPP

#include "Tile.hpp"

// Deklarasi terlebih dahulu
class Property;

class PropertyTile : public Tile {
    private:
        // Atribut
        Property *property;
        int festivalMultiplier;
        int festivalDuration;

    public:
        // Konstruktor dan destruktor
        PropertyTile(const std::string &code, const std::string &name, int pos, Property *prop);
        ~PropertyTile() override;

        // Method untuk menambah status effect festival
        void applyFestivalEffect(int mult, int dur);
        void checkFestivalEffect();
        Property &getProperty();
        const Property &getProperty() const;

        // Override function onLanded dari Tile
        void onLanded(Player &player, GameManager &game) override;
};

#endif
