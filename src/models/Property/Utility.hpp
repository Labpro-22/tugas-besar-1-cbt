#pragma once
#include "Property.hpp"
#include <map>

class Utility : public Property {
    private:
        int buyPrice;
        std::map<int, int> multiplierTable;
        int ownedUtilityCount;
        int lastDiceTotal;

        // Resolve multiplier by owned utility count with safe fallback.
        int resolveMultiplier(int ownedUtilityCount) const;

    public:
        // ctor
        Utility();

        // custom ctor
        Utility(int buyPrice, const std::map<int, int>& multiplierTable);

        // dtor
        ~Utility();

        // Get the purchase price
        int getBuyPrice() const override;

        // Get the property detail information, returns multiplier * dice
        int getPropertyDetail() const override;

        // Set runtime-owned utility count for rent calculation context.
        void setOwnedUtilityCount(int count);

        // Set runtime dice total for rent calculation context.
        void setLastDiceTotal(int diceTotal);

        // Print utility title information
        void printTitle() const override;

        // Utility cannot be demolished
        void demolish() override;

        // Getter methods for buyPrice
        int getPriceValue() const {
            return buyPrice;
        }

        // Getter methods for multiplierTable
        const std::map<int, int>& getMultiplierTable() const {
            return multiplierTable;
        }
        std::string getType() const override {
            return "Utility";
        }
};
