#pragma once
#include <string>
#include "PropertyStatus.hpp"

class Player;

class Property {
    private:
        std::string code;
        std::string name;
        PropertyStatus status;
        int mortgageValue;
        Player* owner;
    protected:
        // Set the owner of this property
        void setOwner(Player* p);
    public:
        // default ctor (status = "BANK")
        Property();

        // custom ctor
        Property(std::string code, std::string name, PropertyStatus status, int mortgageValue, Player* owner);

        // dtor
        virtual ~Property();

        // Get the property code
        std::string getCode() const;

        // Get the property name
        std::string getName() const;

        // Get the current owner of the property, returns nullptr if property is not owned
        Player* getOwner() const;

        // Get the current status of the property (BANK, OWNED, MORTGAGED)
        PropertyStatus getStatus() const;

        // Get the mortgage value of the property
        int getMortgageValue() const;

        // Mortgage the property, returns the mortgage amount given to the player
        virtual int mortgage();

        // Redeem (un-mortgage) the property, requires payment from player.
        virtual void redeem();

        // ---- Must implement functions ----

        // Get the purchase price of the property
        virtual int getBuyPrice() const = 0;

        // Get detailed property information
        virtual int getPropertyDetail() const = 0;

        // Print the title information of the property
        virtual void printTitle() const = 0;

        // Demolish the property
        virtual void demolish() = 0;
};