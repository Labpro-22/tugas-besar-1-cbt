#include "models/Property/Railroad.hpp"
#include "models/GameManager/Player.hpp"
#include <iostream>
#include <iterator>

// ctor
Railroad::Railroad() : Property(), buyPrice(0), rentTable() {}

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
/// @return The rent amount to be paid based on owned railroad count
int Railroad::getPropertyDetail() const {
    if (getOwner() == nullptr) {
        return 0;
    }

    if (rentTable.empty()) {
        return 0;
    }

    int ownedRailroadCount = getOwner()->getRailroadCount();
    if (ownedRailroadCount <= 0) {
        ownedRailroadCount = 1;
    }

    auto exact = rentTable.find(ownedRailroadCount);
    if (exact != rentTable.end()) {
        return exact->second;
    }

    // Fallback to the nearest lower configured tier
    auto upper = rentTable.upper_bound(ownedRailroadCount);
    if (upper == rentTable.begin()) {
        return upper->second;
    }

    return std::prev(upper)->second;
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