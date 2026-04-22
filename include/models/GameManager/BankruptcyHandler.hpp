#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class Player;
class Property;
class LiquidationPanel;

class BankruptcyHandler {
private:
    Player &debtor;
    Player *creditor;
    int debtAmount;
    LiquidationPanel *panel;

public:
    BankruptcyHandler(Player &debtor, Player *creditor, int debt);
    ~BankruptcyHandler();
    int calculateMaxLiquidation();
    bool canCoverDebt();
    bool initiateLiquidation();
    void sellPropertyToBank(Property &prop);
    void mortgageProperty(Property &prop);
    void declareBankrupt();
    void transferAssets();
    void repossessProperties();
    void auctionRepossessedProperties();
};