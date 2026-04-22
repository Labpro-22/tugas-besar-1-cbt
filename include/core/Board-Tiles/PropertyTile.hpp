#ifndef PROPERTY_TILE_HPP
#define PROPERTY_TILE_HPP

#include "Tile.hpp"
#include "../../models/Property/Property.hpp"

// Deklarasi terlebih dahulu
class Property;

class PropertyTile : public Tile {
    private:
        // Atribut
        Property *property;

    public:
        // Konstruktor dan destruktor
        PropertyTile(const std::string &code, const std::string &name, int pos, Property *prop);
        ~PropertyTile() override;

        // Method untuk menambah status effect festival
        void applyFestivalEffect(int mult, int dur);
        Property &getProperty();
        const Property &getProperty() const;

        // Override function onLanded dari Tile
        void onLanded(Player &player, GameManager &game) override;
};

#endif
