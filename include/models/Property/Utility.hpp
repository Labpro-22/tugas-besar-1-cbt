#pragma once
#include "Property.hpp"
#include <map>

class Utility : public Property {
    private:
        int buyPrice;
        std::map<int, int> multiplierTable;

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

        // Print utility title information
        void printTitle() const override;

        // Utility cannot be demolished
        void demolish() override;

        // Getter methods for buyPrice
        int getPriceValue() const {
            return buyPrice;
        }

        // Getter methods for multiplierTable
        std::map<int, int> getMultiplierTable() const {
            return multiplierTable;
        }
        std::string getType() const override {
            return "Utility";
        }
};
