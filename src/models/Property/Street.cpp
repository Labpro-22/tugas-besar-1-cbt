#include "models/Property/Street.hpp"
#include "models/GameManager/Player.hpp"
#include "exception/NimonspoliExceptions.hpp"
#include <algorithm>
#include <iostream>

// ctor
Street::Street()
    : Property(), buyPrice(0), color(ColorGroup::COKLAT), houseCost(0),
        hotelCost(0), level(BuildingLevel::EMPTY) {
    rentLevels.resize(6, 0);
}

// custom ctor
/// @param buyPrice The purchase price of the street
/// @param color The color group of this street
/// @param rentLevels Vector of rent prices for each building level
/// @param houseCost The cost to build a house on this street
/// @param hotelCost The cost to upgrade to a hotel on this street
/// @param level The current building level.
/// @param festivalMultiplier The event multiplier for special effects
Street::Street(int buyPrice, ColorGroup color, std::vector<int> rentLevels,
    int houseCost, int hotelCost, BuildingLevel level,
    int festivalMultiplier)
    : Property(), buyPrice(buyPrice), color(color), rentLevels(rentLevels),
        houseCost(houseCost), hotelCost(hotelCost), level(level) {
    setFestival(festivalMultiplier, 0);
}

// dtor
Street::~Street() {}

// Get the purchase price
/// @return The buy price.
int Street::getBuyPrice() const { return buyPrice; }

// Get property detail
/// @return The rent amount to be paid based on current building level
int Street::getPropertyDetail() const {
    if (getOwner() == nullptr) {
        return 0;
    }

    if (rentLevels.empty()) {
        throw PropertyBuildException(getCode(), "Daftar sewa street kosong.");
    }

    if (!rentLevels.empty() &&
        static_cast<int>(level) < static_cast<int>(rentLevels.size())) {
        int rent = rentLevels[static_cast<int>(level)];
        if (level == BuildingLevel::EMPTY && isMonopolized()) {
            rent *= 2;
        }
        return rent;
    }

    throw PropertyBuildException(getCode(), "Level bangunan di luar daftar sewa.");
}

/// Print the title information of this street
void Street::printTitle() const {
    std::cout << "=== Street Title ===" << std::endl;
    std::cout << "Code: " << getCode() << std::endl;
    std::cout << "Name: " << getName() << std::endl;
    std::cout << "Buy Price: " << buyPrice << std::endl;
    std::cout << "House Cost: " << houseCost << std::endl;
    std::cout << "Hotel Cost: " << hotelCost << std::endl;

    std::cout << "Rent Levels: ";
    for (int rent : rentLevels) {
        std::cout << rent << " ";
    }
    std::cout << std::endl;

    if (getOwner() != nullptr) {
        std::cout << "Owner: Player (ID: " << getOwner() << ")" << std::endl;
    } else {
        std::cout << "Owner: Bank (unowned)" << std::endl;
    }

    std::cout << "Building Level: " << static_cast<int>(level) << std::endl;
    std::cout << "===================" << std::endl;
}

// Upgrade this street to hotel level if not already.
void Street::upgradeHotel() {
    if (level < BuildingLevel::HOTEL) {
        level = BuildingLevel::HOTEL;
    }
}

// Check if all streets of the same color are owned by the same player
// (monopoly).
/// @return true if this street's color group is monopolized, false otherwise.
bool Street::isMonopolized() const {
    Player *owner = getOwner();
    if (owner == nullptr) {
        return false;
    }

    int sameColorOwned = 0;
    for (Property *prop : owner->getProperties()) {
        if (prop != nullptr && prop->getType() == "Street") {
            const Street *street = static_cast<const Street *>(prop);
            if (street->getColorGroup() == color) {
                ++sameColorOwned;
            }
        }
    }

    if (color == ColorGroup::COKLAT || color == ColorGroup::BIRU_TUA) {
        return sameColorOwned >= 2;
    }

    return sameColorOwned >= 3;
}

// Activate special event effects on this street
/// @param multiplier The factor to apply to rent or other calculations.
void Street::activateEffect(int multiplier) {
    if (multiplier <= 0) {
        throw PropertyBuildException(getCode(), "Multiplier festival harus positif.");
    }

    if (multiplier > 0) {
        setFestival(multiplier, getFDur());
    }
}

// Demolish all buildings on this street
void Street::demolish() {
    level = BuildingLevel::EMPTY;
    festivalMultiplier = 1;
}

int Street::getBuildingInvestmentValue() const {
    const int buildingCount = getBuildingCount();
    if (buildingCount <= 0) {
        return 0;
    }

    if (buildingCount >= static_cast<int>(BuildingLevel::HOTEL)) {
        return (4 * houseCost) + hotelCost;
    }

    return buildingCount * houseCost;
}

int Street::getNextBuildCost() const {
    if (level == BuildingLevel::HOTEL) {
        return 0;
    }

    if (level == BuildingLevel::FOUR_HOUSE) {
        return hotelCost;
    }

    return houseCost;
}

bool Street::canBuildNext() const {
    return getStatus() == PropertyStatus::OWNED && level < BuildingLevel::HOTEL;
}

bool Street::isNextBuildHotel() const {
    return level == BuildingLevel::FOUR_HOUSE;
}

void Street::buildNext() {
    if (!canBuildNext()) {
        return;
    }

    level = static_cast<BuildingLevel>(static_cast<int>(level) + 1);
}