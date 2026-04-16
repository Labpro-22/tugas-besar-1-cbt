#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Player.hpp"
#include "../Property/Property.hpp"
using namespace std;


class LiquidationPanel {
    private :
        Player* debtor;
        int debtAmount;
        int remainingDebt;
        vector<Property*> sellableProperties;
        vector<Property*> mortgageableProperties;
    public:
        LiquidationPanel(Player* p, int initialDebt);
        vector<Property*> getSellableProperties();
        vector<Property*> getMortgageableProperties();
        bool sellToBank(Property* prop);
        bool mortgage(Property* prop);
        bool isDebtSatisfied();
        int getRemainingDebt();  
};