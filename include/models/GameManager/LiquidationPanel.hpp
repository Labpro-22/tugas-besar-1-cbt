#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class Player;
class Property;

class LiquidationPanel {
private:
    Player *debtor;
    int debtAmount;
    int remainingDebt;
    vector<Property *> sellableProperties;
    vector<Property *> mortgageableProperties;

public:
    LiquidationPanel(Player *p, int initialDebt);
    vector<Property *> getSellableProperties();
    vector<Property *> getMortgageableProperties();
    bool sellToBank(Property *prop);
    bool mortgage(Property *prop);
    bool isDebtSatisfied();
    int getRemainingDebt();
};