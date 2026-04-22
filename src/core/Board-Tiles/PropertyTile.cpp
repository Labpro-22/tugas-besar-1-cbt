#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "exception/NimonspoliExceptions.hpp"

PropertyTile::PropertyTile(const std::string &code, const std::string &name,
                           int pos, Property *prop)
    : Tile(code, name, pos, "property"), property(prop) {
    if (property == nullptr) {
        throw PropertyUnavailableException(code);
    }
}

PropertyTile::~PropertyTile() {
        delete property;
        property = nullptr;
}

void PropertyTile::applyFestivalEffect(int mult, int dur) {
    if (!(mult == 1 || mult == 2 || mult == 4 || mult == 8)) {
        throw InvalidInputException("Festival multiplier harus 1,2,4,atau 8.",
                                    "INVALID_FESTIVAL_MULTIPLIER");
    }
    if (dur < 0 || dur > 3) {
        throw InvalidInputException("Durasi festival harus 0..3.",
                                    "INVALID_FESTIVAL_DURATION");
    }
    if (property == nullptr) {
        throw PropertyUnavailableException(getCode());
    }
    property->setFestival(mult, dur);
}

Property &PropertyTile::getProperty() {
    if (property == nullptr) {
        throw PropertyUnavailableException(getCode());
    }
    return *property;
}

const Property &PropertyTile::getProperty() const {
    if (property == nullptr) {
        throw PropertyUnavailableException(getCode());
    }
    return *property;
}

void PropertyTile::onLanded(Player &player, GameManager &game) {
    try {
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
    } catch (const NimonspoliException &) {
        throw;
    } catch (const std::exception &e) {
        throw InternalGameException(std::string("PropertyTile::onLanded: ") + e.what());
    }
}
