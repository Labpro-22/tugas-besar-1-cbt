#pragma once
#include "PropertyStatus.hpp"
#include <string>

class Player;

class Property {
private:
    std::string code;
    std::string name;
    PropertyStatus status;
    int mortgageValue;
    Player *owner;
    int festivalMultiplier;
    int festivalDuration;

public:
    // default ctor (status = "BANK")
    Property();

    // custom ctor
    Property(std::string code, std::string name, PropertyStatus status,
        int mortgageValue, Player *owner);

    // dtor
    virtual ~Property();

    // Get the property code
    std::string getCode() const;

    // Get the property name
    std::string getName() const;

    // Get the current owner of the property, returns nullptr if property is not owned
    Player *getOwner() const;

    // Get the current status of the property (BANK, OWNED, MORTGAGED)
    PropertyStatus getStatus() const;

    // Get the mortgage value of the property
    int getMortgageValue() const;

    // Set the owner of this property
    void setOwner(Player *p);

    virtual std::string getType() const = 0;
    std::string getStatusString() const;
    virtual int getFMult() const;
    virtual int getFDur() const;

    virtual int getBuildingCount() const { return 0; }
    virtual int getBuildingInvestmentValue() const { return 0; }
    virtual int getBuildingSellValue() const { return getBuildingInvestmentValue() / 2; }
    virtual std::string getBuildingLabel() const { return "-"; }

    void setCode(std::string newCode);
    void setName(std::string newName);
    void setMortgageValue(int value);
    void setStatusStr(std::string statusStr);
    virtual void setFestival(int multiplier, int duration);
    virtual void tickFestival();
    virtual void setBuildingCount(int) {}
    virtual bool canBeMortgaged() const;
    virtual void resetToBank();

    // Mortgage the property, returns the mortgage amount given to the player
    virtual int mortgage();

    // Get the amount required to redeem this property.
    virtual int getRedeemPrice() const;

    // Redeem (un-mortgage) the property, returns the required redeem amount.
    virtual int redeem();

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
