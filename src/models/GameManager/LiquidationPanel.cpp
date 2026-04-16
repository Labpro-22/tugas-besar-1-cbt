#include "LiquidationPanel.hpp"
#include <algorithm> 
using namespace std;

LiquidationPanel::LiquidationPanel(Player* p, int initialDebt) : debtor(p), debtAmount(initialDebt), remainingDebt(initialDebt) {
    if (debtor != nullptr) {
        for (Property* prop : debtor->getProperties()) {
            sellableProperties.push_back(prop);
            mortgageableProperties.push_back(prop);
        }
    }
}

vector<Property*> LiquidationPanel::getSellableProperties() {
    return sellableProperties;
}

vector<Property*> LiquidationPanel::getMortgageableProperties() {
    return mortgageableProperties;
}

bool LiquidationPanel::sellToBank(Property* prop) {
    if (debtor == nullptr){
        return false;
    }

    auto it = find(sellableProperties.begin(), sellableProperties.end(), prop);
    
    if (it != sellableProperties.end()) {
        sellableProperties.erase(it);
        auto itMortgage = find(mortgageableProperties.begin(), mortgageableProperties.end(), prop);
        if (itMortgage != mortgageableProperties.end()) {
            mortgageableProperties.erase(itMortgage);
        }

        int sellValue = prop->getBuyPrice(); 
        
        if(remainingDebt - sellValue > 0){
            debtor->addCash(remainingDebt-sellValue);
        }
        remainingDebt -= sellValue;
        
        debtor->removeProperty(prop);
        prop->setOwner(nullptr);    

        return true;
    }
    return false;
}

bool LiquidationPanel::mortgage(Property* prop) {
    if (debtor == nullptr) {
        return false;
    }
    auto it = find(mortgageableProperties.begin(), mortgageableProperties.end(), prop);
    
    if (it != mortgageableProperties.end()) {
        mortgageableProperties.erase(it);

        int mortgageValue = prop->getMortgageValue();
        
        debtor->addCash(mortgageValue);
        remainingDebt -= mortgageValue;  
        prop->setStatus(BANK); 

        return true;
    }
    return false;
}

bool LiquidationPanel::isDebtSatisfied() {
    if (debtor == nullptr) return false;
    return (debtor->getCash() >= remainingDebt) || (remainingDebt <= 0);
}


int LiquidationPanel::getRemainingDebt() {
    if (debtor == nullptr){
        return 0;
    }
    int needed = remainingDebt - debtor->getCash();
    if(needed < 0){
        return needed;
    }
    return 0;
}
