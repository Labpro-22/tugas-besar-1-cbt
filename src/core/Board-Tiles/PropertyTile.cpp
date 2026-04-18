#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"

PropertyTile::PropertyTile(const std::string &code, const std::string &name,
                                                                                                            int pos, Property *prop)
                : Tile(code, name, pos, "property"), property(prop), festivalMultiplier(1),
                        festivalDuration(0) {}

PropertyTile::~PropertyTile() {
        delete property;
        property = nullptr;
}

void PropertyTile::applyFestivalEffect(int mult, int dur) {
        festivalMultiplier = mult;
        festivalDuration = dur;
}

void PropertyTile::checkFestivalEffect() {
        if (festivalDuration > 0) {
                festivalDuration--;
                if (festivalDuration == 0) {
                        festivalMultiplier = 1;
                }
        }
}

Property &PropertyTile::getProperty() { return *property; }

void PropertyTile::onLanded(Player &player, GameManager &game) {
        if (property == nullptr) {
                return;
        }

        checkFestivalEffect();

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
        if (festivalMultiplier > 1) {
                rentDue *= festivalMultiplier;
        }

        if (!player.canPay(rentDue)) {
                Player *creditor = owner;
                game.executeBankruptcy(player, creditor, rentDue);
                return;
        }

        game.executeRentPayer(player, *owner, prop, rentDue);
}
