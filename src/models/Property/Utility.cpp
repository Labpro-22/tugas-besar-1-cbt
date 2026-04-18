#include "Utility.hpp"
#include <iostream>

// ctor
Utility::Utility() : Property(), buyPrice(0) {
    // Default multiplier table: key = owned utility count, value = rent multiplier
    multiplierTable[1] = 4;
    multiplierTable[2] = 10;
}

// custom ctor
/// @param buyPrice The purchase price of the street
/// @param multiplierTable The multiplier of rent price
Utility::Utility(int buyPrice, const std::map<int, int>& multiplierTable)
    : Property(), buyPrice(buyPrice), multiplierTable(multiplierTable) {}

// dtor
Utility::~Utility() {}

// Resolve multiplier by owned utility count
int Utility::resolveMultiplier(int ownedUtilityCount) const {
    if (multiplierTable.empty()) {
        return 0;
    }

    auto exact = multiplierTable.find(ownedUtilityCount);
    if (exact != multiplierTable.end()) {
        return exact->second;
    }

    auto singleOwned = multiplierTable.find(1);
    if (singleOwned != multiplierTable.end()) {
        return singleOwned->second;
    }

    return multiplierTable.begin()->second;
}

// Get the purchase price
int Utility::getBuyPrice() const {
    return buyPrice;
}

// Get property detail
int Utility::getPropertyDetail() const {
    if (getOwner() == nullptr) {
        return 0;
    }

    // later
    int ownedUtilityCount = 1;
    int multiplier = resolveMultiplier(ownedUtilityCount);

    // later
    int diceTotal = 0;
    return diceTotal * multiplier;
}

// Print utility title information
void Utility::printTitle() const {
    std::cout << "=== Utility Title ===" << std::endl;
    std::cout << "Code: " << getCode() << std::endl;
    std::cout << "Name: " << getName() << std::endl;
    std::cout << "Buy Price: " << buyPrice << std::endl;

    std::cout << "Multiplier Table: ";
    for (const auto& [ownedCount, multiplier] : multiplierTable) {
        std::cout << "(" << ownedCount << "->" << multiplier << ") ";
    }
    std::cout << std::endl;

    if (getOwner() != nullptr) {
        std::cout << "Owner: Player (ptr: " << getOwner() << ")" << std::endl;
    } else {
        std::cout << "Owner: Bank (unowned)" << std::endl;
    }

    std::cout << "=====================" << std::endl;
}

// Utility cannot be demolished 
void Utility::demolish() {
    // nothing
}

std::string Utility::getType() const {
    return "Utility";
}