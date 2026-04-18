#include "Railroad.hpp"
#include <iostream>

// ctor
Railroad::Railroad() : Property(), buyPrice(0), ownedRailroadCount(1) {
    // Default rent table: key = number of railroad owned, value = rent.
    rentTable[1] = 25;
    rentTable[2] = 50;
    rentTable[3] = 100;
    rentTable[4] = 200;
}

// custom ctor
/// @param buyPrice The purchase price of the railroad
/// @param rentTable The rule of rent for railroad
Railroad::Railroad(int buyPrice, const std::map<int, int>& rentTable)
    : Property(), buyPrice(buyPrice), rentTable(rentTable), ownedRailroadCount(1) {}

// dtor
Railroad::~Railroad() {}

// Get the purchase price 
/// @return The buy price.
int Railroad::getBuyPrice() const {
    return buyPrice;
}

// Get property detail 
/// @return The rent amount to be paid based on owned railroad count (fallback to first entry).
int Railroad::getPropertyDetail() const {
    if (getOwner() == nullptr) {
        return 0; // Unowned property doesn't charge rent
    }

    return resolveRent(ownedRailroadCount);
}

// Resolve rent by owned railroad count with safe fallback.
int Railroad::resolveRent(int ownedCount) const {
    if (rentTable.empty()) {
        return 0;
    }

    auto exact = rentTable.find(ownedCount);
    if (exact != rentTable.end()) {
        return exact->second;
    }

    auto singleRailroadRent = rentTable.find(1);
    if (singleRailroadRent != rentTable.end()) {
        return singleRailroadRent->second;
    }

    return rentTable.begin()->second;
}

// Set runtime-owned railroad count for rent calculation context.
void Railroad::setOwnedRailroadCount(int count) {
    if (count < 1) {
        ownedRailroadCount = 1;
        return;
    }

    ownedRailroadCount = count;
}

// Print the title information of this Railroad
void Railroad::printTitle() const {
    std::cout << "=== Railroad Title ===" << std::endl;
    std::cout << "Code: " << getCode() << std::endl;
    std::cout << "Name: " << getName() << std::endl;
    std::cout << "Buy Price: " << buyPrice << std::endl;

    std::cout << "Rent Table: ";
    for (const auto& [ownedCount, rent] : rentTable) {
        std::cout << "(" << ownedCount << "->" << rent << ") ";
    }
    std::cout << std::endl;

    if (getOwner() != nullptr) {
        std::cout << "Owner: Player (ptr: " << getOwner() << ")" << std::endl;
    } else {
        std::cout << "Owner: Bank (unowned)" << std::endl;
    }

    std::cout << "===================" << std::endl;
}

// Railroad cannot be demolished
void Railroad::demolish() {
    // nothing
}