#pragma once
#include "Property.hpp"
#include <map>

class Railroad : public Property {
    private:
        int buyPrice;
        std::map<int, int> rentTable;
    public:
        // ctor 
        Railroad();

        // custom ctor
        Railroad(int buyPrice, std::map<int, int> rentTable);

        // dtor
        ~Railroad();

        // Get the purchase price
        int getBuyPrice() const override;

        // Get the property detail information, returns rent based on rentTable
        int getPropertyDetail() const override;

        // Print the title information of this railroad
        void printTitle() const override;

        // Railroad cannot be demolished
        void demolish() override;

        // Getter methods for buyPrice
        int getPriceValue() const { 
            return buyPrice; 
        }

        // Getter methods for rentTable
        std::map<int, int> getRentTable() const {
            return rentTable;
        }
};