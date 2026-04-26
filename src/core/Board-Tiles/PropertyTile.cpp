#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "exception/NimonspoliExceptions.hpp"
#include <algorithm>

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
            const int basePrice = prop.getBuyPrice();
            const int price = game.applyDiscount(player, basePrice);
            std::string detail = "Mendarat di " + prop.getName() + " (" + prop.getCode() +
                                 "). Tersedia untuk dibeli seharga M" + std::to_string(basePrice);
            if (price != basePrice) {
                detail += " (diskon -> M" + std::to_string(price) + ")";
            }
            detail += ". Uang kamu: M" + std::to_string(player.getCash()) +
                      ", tersisa M" + std::to_string(player.getCash() - price) + " jika dibeli.";
            logTileEvent(game, player, "PROPERTI", detail);
            game.executePurchase(player, prop);
            return;
        }

        if (owner == &player) {
            logTileEvent(game, player, "PROPERTI",
                         "Mendarat di " + prop.getName() + " (" + prop.getCode() +
                             ") - properti milik sendiri.");
            return;
        }

        if (prop.getStatus() == PropertyStatus::MORTGAGED) {
            logTileEvent(game, player, "PROPERTI",
                         "Mendarat di " + prop.getName() + " (" + prop.getCode() +
                             ") - properti tergadai, tidak ada sewa.");
            return;
        }

        int rentDue = prop.getPropertyDetail();
        if (prop.getType() == "Utility") {
            const int diceTotal = std::max(1, game.getLastDiceTotal());
            rentDue *= diceTotal;
        }
        if (prop.getFMult() > 1) {
                rentDue *= prop.getFMult();
        }

        std::string rentDetail = "Mendarat di " + prop.getName() + " (" + prop.getCode() +
                                 "), milik " + owner->getUsername() +
                                 ". Sewa: M" + std::to_string(rentDue);
        if (prop.getFMult() > 1) {
            rentDetail += " (festival x" + std::to_string(prop.getFMult()) + ")";
        }
        logTileEvent(game, player, "PROPERTI", rentDetail);

        game.executeRentPayer(player, *owner, prop, rentDue);
    } catch (const NimonspoliException &) {
        throw;
    } catch (const std::exception &e) {
        throw InternalGameException(std::string("PropertyTile::onLanded: ") + e.what());
    }
}
