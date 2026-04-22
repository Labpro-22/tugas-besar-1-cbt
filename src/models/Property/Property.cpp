#include "models/Property/Property.hpp"
#include <iostream>

// default ctor (status = "BANK")
Property::Property()
    : code(""), name(""), status(PropertyStatus::BANK), mortgageValue(0),
        owner(nullptr), festivalMultiplier(1), festivalDuration(0) {}

// custom ctor
/// @param code The property code
/// @param name The name of the property
/// @param status The initial status (BANK, OWNED, or MORTGAGED)
/// @param mortgageValue The value of the property mortgage
/// @param owner Pointer to the player who owns the property
Property::Property(std::string code, std::string name, PropertyStatus status,
    int mortgageValue, Player *owner)
    : code(code), name(name), status(status), mortgageValue(mortgageValue),
        owner(owner), festivalMultiplier(1), festivalDuration(0) {}

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

int Property::getFMult() const { return festivalMultiplier; }

int Property::getFDur() const { return festivalDuration; }

void Property::setFestival(int multiplier, int duration) {
    festivalMultiplier = multiplier > 1 ? multiplier : 1;
    festivalDuration = duration > 0 && festivalMultiplier > 1 ? duration : 0;
    if (festivalDuration == 0) {
        festivalMultiplier = 1;
    }
}

void Property::tickFestival() {
    if (festivalDuration <= 0) {
        festivalMultiplier = 1;
        festivalDuration = 0;
        return;
    }

    festivalDuration--;
    if (festivalDuration == 0) {
        festivalMultiplier = 1;
    }
}

bool Property::canBeMortgaged() const {
    return status == PropertyStatus::OWNED && getBuildingCount() == 0;
}

void Property::resetToBank() {
    owner = nullptr;
    status = PropertyStatus::BANK;
    setFestival(1, 0);
    setBuildingCount(0);
}

// Mortgage the property, transition status from OWNED to MORTGAGED
/// @return The mortgage amount given to the player
int Property::mortgage() {
    if (canBeMortgaged()) {
        status = PropertyStatus::MORTGAGED;
        return mortgageValue;
    }
    return 0;
}

int Property::getRedeemPrice() const {
    return getBuyPrice();
}

// Redeem (un-mortgage) the property, transition status from MORTGAGED back to OWNED
/// @note Player must pay back the full buy price to redeem
/// @return The redeem price charged for this property
int Property::redeem() {
    if (status != PropertyStatus::MORTGAGED) {
        return 0;
    }

    const int redeemPrice = getRedeemPrice();

    // Keep object state consistent: a redeemed property should be owned by a player.
    status = (owner != nullptr) ? PropertyStatus::OWNED : PropertyStatus::BANK;
    return redeemPrice;
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
