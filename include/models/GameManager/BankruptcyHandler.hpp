#pragma once
#include <iostream>
#include <string>
#include <vector>

class Property;
class Player;

using namespace std;

class BankruptcyHandler {
private:
    Player *debtor;
    Player *creditor;
    int debtAmount;
    vector<Property *> sellableProperties;
    vector<Property *> mortgageableProperties;

    void buildAssetLists();
    bool sellToBank(Property *prop);
    bool mortgageProperty(Property *prop);
    bool isDebtSatisfied() const;

public:
    BankruptcyHandler(Player *debtor, Player *creditor, int debt);
    int calculateMaxLiquidation();
    bool canCoverDebt();
    bool initiateLiquidation();
    vector<Property*> declareBankrupt();
    void transferAssets();
    vector<Property *> repossessProperties();
};
