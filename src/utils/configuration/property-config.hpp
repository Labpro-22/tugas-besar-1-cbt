#ifndef PROPERTY_CONFIG_HPP
#define PROPERTY_CONFIG_HPP

#include <iostream>
using namespace std;

class PropertyConfig {
    private:
        int id;
        int code;
        int name;
        int type;
        int color;
        int landPrice;
        int mortgageValue;
        int houseCost;
        int hotelCost;
        // Rent for 0-5 houses and hotel, 
        // index 0 is rent for land, index 5 is rent with hotel
        // 0-based indexing (actual and logical).
        int rent[6];
    public:
        PropertyConfig();
        int getPurchasePrice() const;
        const int* getRent() const;
        const int getID() const;
        void setPropertyAmount(int purchasePrice, int rent);

        friend std::istream& operator>>(std::istream& is, PropertyConfig& config);
};

#endif // PROPERTY_CONFIG_HPP