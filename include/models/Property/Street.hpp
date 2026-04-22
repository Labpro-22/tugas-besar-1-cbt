#pragma once
#include "BuildingLevel.hpp"
#include "ColorGroup.hpp"
#include "Property.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

class Street : public Property {
private:
    int buyPrice;
    ColorGroup color;
    std::vector<int> rentLevels;
    int houseCost;
    int hotelCost;
    BuildingLevel level;

public:
    // ctor (default color COKLAT)
    Street();

    // custom ctor
    Street(int buyPrice, ColorGroup color, std::vector<int> rentLevels,
        int houseCost, int hotelCost, BuildingLevel level,
        int festivalMultiplier);

    // dtor
    ~Street();

    // Get the purchase price
    int getBuyPrice() const override;

    // Get the property detail information, returns rent based on building level
    int getPropertyDetail() const override;

    // Print the title information of this street
    void printTitle() const override;

    // Upgrade this street to hotel level
    void upgradeHotel();

    // Check if this street is monopolized (all same color owned by one player)
    bool isMonopolized() const;

    // Activate any special effect on this street
    void activateEffect(int multiplier);

    // Demolish buildings from this street
    void demolish() override;

    int getBuildingInvestmentValue() const override;
    int getNextBuildCost() const;
    bool canBuildNext() const;
    bool isNextBuildHotel() const;
    void buildNext();

    // Getter methods for buyPrice
    int getPriceValue() const { return buyPrice; }

    // Get the color group of this street
    ColorGroup getColorGroup() const { return color; }

    // Get the house cost
    int getHouseCost() const { return houseCost; }

    // Get the hotel cost
    int getHotelCost() const { return hotelCost; }

    // Get the current building level
    BuildingLevel getBuildingLevel() const { return level; }

    // Get the rent levels vector
    const std::vector<int> &getRentLevels() const { return rentLevels; }
    std::string getType() const override { return "Street"; }

    int getBuildingCount() const override { return static_cast<int>(level); }

    void setBuildingCount(int count) override {
        level = static_cast<BuildingLevel>(count);
    }
};
