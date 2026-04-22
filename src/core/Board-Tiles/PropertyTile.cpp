#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"

PropertyTile::PropertyTile(const std::string &code, const std::string &name,
                                                                                                            int pos, Property *prop)
                : Tile(code, name, pos, "property"), property(prop) {}

PropertyTile::~PropertyTile() {
        delete property;
        property = nullptr;
}

void PropertyTile::applyFestivalEffect(int mult, int dur) {
        if (property != nullptr) {
                property->setFestival(mult, dur);
        }
}

Property &PropertyTile::getProperty() { return *property; }

const Property &PropertyTile::getProperty() const { return *property; }

void PropertyTile::onLanded(Player &player, GameManager &game) {
        if (property == nullptr) {
                return;
        }

        Property &prop = getProperty();
        Player *owner = prop.getOwner();

        if (owner == nullptr) {
                game.executePurchase(player, prop);
                return;
        }

        if (owner == &player) {
                return;
        }

        if (prop.getStatus() == PropertyStatus::MORTGAGED) {
                return;
        }

        int rentDue = prop.getPropertyDetail();
        if (prop.getFMult() > 1) {
                rentDue *= prop.getFMult();
        }

        game.executeRentPayer(player, *owner, prop, rentDue);
}
