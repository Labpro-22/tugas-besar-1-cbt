#include "../../../include/models/Property/Street.hpp"
#include "../../../include/models/GameManager/Player.hpp"
#include <algorithm>
#include <iostream>

// ctor
Street::Street()
    : Property(), buyPrice(0), color(ColorGroup::COKLAT), houseCost(0),
        hotelCost(0), level(BuildingLevel::EMPTY), festivalMultiplier(1) {
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
        houseCost(houseCost), hotelCost(hotelCost), level(level),
        festivalMultiplier(festivalMultiplier) {}

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

    if (!rentLevels.empty() &&
        static_cast<int>(level) < static_cast<int>(rentLevels.size())) {
        return rentLevels[static_cast<int>(level)];
    }

    return 0;
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
        Street *street = dynamic_cast<Street *>(prop);
        if (street != nullptr && street->getColorGroup() == color) {
            ++sameColorOwned;
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
    if (multiplier > 0) {
        festivalMultiplier = multiplier;
    }
}

// Demolish all buildings on this street
void Street::demolish() {
    level = BuildingLevel::EMPTY;
    festivalMultiplier = 1;
}