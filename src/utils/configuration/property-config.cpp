#include "property-config.hpp"

PropertyConfig::PropertyConfig() {
    id = 0;
    code = 0;
    name = 0;
    type = 0;
    color = 0;
    landPrice = 0;
    mortgageValue = 0;
    houseCost = 0;
    hotelCost = 0;
    for (int i = 0; i < 6; i++) {
        rent[i] = 0;
    }
}

int PropertyConfig::getPurchasePrice() const {
    return landPrice;
}

const int* PropertyConfig::getRent() const {
    return rent;
}

void PropertyConfig::setPropertyAmount(int purchasePrice, int rent) {
    landPrice = purchasePrice;
    this->rent[0] = rent;
}

std::istream& operator>>(std::istream& is, PropertyConfig& config) {
    is >> config.id >> config.code >> config.name >> config.type >> config.color
       >> config.landPrice >> config.mortgageValue >> config.houseCost >> config.hotelCost;
    for (int i = 0; i < 6; i++) {
        is >> config.rent[i];
    }
    return is;
}

const int PropertyConfig::getID() const {
    return id;
}

const int PropertyConfig::getCode() const {
    return code;
}

int PropertyConfig::getName() const {
    return name;
}

int PropertyConfig::getType() const {
    return type;
}

int PropertyConfig::getColor() const {
    return color;
}

int PropertyConfig::getLandPrice() const {
    return landPrice;
}

int PropertyConfig::getMortgageValue() const {
    return mortgageValue;
}

int PropertyConfig::getHouseCost() const {
    return houseCost;
}

int PropertyConfig::getHotelCost() const {
    return hotelCost;
}
