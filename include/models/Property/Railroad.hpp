#pragma once
#include "Property.hpp"
#include <map>

class Railroad : public Property {
    private:
        int buyPrice;
        std::map<int, int> rentTable;
        int ownedRailroadCount;

        // Resolve rent by owned railroad count with safe fallback.
        int resolveRent(int ownedCount) const;
    public:
        // ctor 
        Railroad();

        // custom ctor
        Railroad(int buyPrice, const std::map<int, int>& rentTable);

        // dtor
        ~Railroad();

        // Get the purchase price
        int getBuyPrice() const override;

        // Get the property detail information, returns rent based on rentTable
        int getPropertyDetail() const override;

        // Set runtime-owned railroad count for rent calculation context.
        void setOwnedRailroadCount(int count);

        // Print the title information of this railroad
        void printTitle() const override;

        // Railroad cannot be demolished
        void demolish() override;

        // Getter methods for buyPrice
        int getPriceValue() const { 
            return buyPrice; 
        }
        std::string getType() const override {
            return "Railroad";
        }
        // Getter methods for rentTable
        const std::map<int, int>& getRentTable() const {
            return rentTable;
        }
};