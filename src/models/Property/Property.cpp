#include "Property.hpp"
#include <iostream>

// default ctor (status = "BANK")
Property::Property()
    : code(""), name(""), status(PropertyStatus::BANK), mortgageValue(0),
        owner(nullptr) {}

// custom ctor
/// @param code The property code
/// @param name The name of the property
/// @param status The initial status (BANK, OWNED, or MORTGAGED)
/// @param mortgageValue The value of the property mortgage
/// @param owner Pointer to the player who owns the property
Property::Property(std::string code, std::string name, PropertyStatus status,
    int mortgageValue, Player *owner)
    : code(code), name(name), status(status), mortgageValue(mortgageValue),
        owner(owner) {}

// dtor
Property::~Property() {}

// Get the property code
std::string Property::getCode() const { return code; }

// Get the property name
std::string Property::getName() const { return name; }

// Get the current owner of the property
/// @return Pointer to the Player who owns this property, or nullptr if not owned
Player *Property::getOwner() const { return owner; }

// Get the current status of the property
/// @return PropertyStatus::BANK (owned by bank), OWNED (owned by player), or MORTGAGED (mortgaged)
PropertyStatus Property::getStatus() const { return status; }

// Get the mortgage value of the property
int Property::getMortgageValue() const { return mortgageValue; }

// Set the owner of this property
/// @param p Pointer to the player who is taking ownership
void Property::setOwner(Player *p) { owner = p; }

// Mortgage the property, transition status from OWNED to MORTGAGED
/// @return The mortgage amount given to the player
int Property::mortgage() {
    if (status == PropertyStatus::OWNED) {
        status = PropertyStatus::MORTGAGED;
        return mortgageValue;
    }
    return 0;
}

// Redeem (un-mortgage) the property, transition status from MORTGAGED back to OWNED
/// @note Player must pay back mortgage value plus interest to redeem.
void Property::redeem() {
    if (status == PropertyStatus::MORTGAGED) {
        status = PropertyStatus::OWNED;
    }

    // later
}

void Property::setCode(std::string newCode) { code = newCode; }

void Property::setName(std::string newName) { name = newName; }

void Property::setMortgageValue(int value) { mortgageValue = value; }

std::string Property::getStatusString() const {
    if (status == PropertyStatus::BANK)
        return "BANK";
    if (status == PropertyStatus::OWNED)
        return "OWNED";
    if (status == PropertyStatus::MORTGAGED)
        return "MORTGAGED";
    return "BANK";
}

void Property::setStatusStr(std::string statusStr) {
    if (statusStr == "BANK")
        status = PropertyStatus::BANK;
    else if (statusStr == "OWNED")
        status = PropertyStatus::OWNED;
    else if (statusStr == "MORTGAGED")
        status = PropertyStatus::MORTGAGED;
}
