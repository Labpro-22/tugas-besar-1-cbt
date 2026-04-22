#include "../../../include/models/Property/Railroad.hpp"
#include <iostream>

// ctor
Railroad::Railroad() : Property(), buyPrice(0) {
    // Default rent table: key = number of railroad owned, value = rent.
    rentTable[1] = 25;
    rentTable[2] = 50;
    rentTable[3] = 100;
    rentTable[4] = 200;
}

// custom ctor
/// @param buyPrice The purchase price of the railroad
/// @param rentTable The rule of rent for railroad
Railroad::Railroad(int buyPrice, std::map<int, int> rentTable)
    : Property(), buyPrice(buyPrice), rentTable(rentTable) {}

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
        return 0;
    }

    if (rentTable.empty()) {
        return 0;
    }

    // placeholder
    auto singleRailroadRent = rentTable.find(1);
    if (singleRailroadRent != rentTable.end()) {
        return singleRailroadRent->second;
    }

    return rentTable.begin()->second;
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