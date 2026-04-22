#include "../../../include/models/GameManager/LiquidationPanel.hpp"
#include "../../../include/models/Property/Property.hpp"
#include "../../../include/models/Property/Street.hpp"
#include "../../../include/models/GameManager/Player.hpp"
#include <algorithm>
using namespace std;

LiquidationPanel::LiquidationPanel(Player *p, int initialDebt)
    : debtor(p), debtAmount(initialDebt), remainingDebt(initialDebt) {
    if (debtor != nullptr) {
        for (Property *prop : debtor->getProperties()) {
            if (prop == nullptr) {
                continue;
            }
            if (prop->getStatusString() == "OWNED") {
                sellableProperties.push_back(prop);
                if (prop->getBuildingCount() == 0) {
                    mortgageableProperties.push_back(prop);
                }
            }
        }
    }
}

vector<Property *> LiquidationPanel::getSellableProperties() {
    return sellableProperties;
}

vector<Property *> LiquidationPanel::getMortgageableProperties() {
    return mortgageableProperties;
}

bool LiquidationPanel::sellToBank(Property *prop) {
    if (debtor == nullptr) {
        return false;
    }

    auto it = find(sellableProperties.begin(), sellableProperties.end(), prop);

    if (it != sellableProperties.end()) {
        sellableProperties.erase(it);
        auto itMortgage = find(mortgageableProperties.begin(),
            mortgageableProperties.end(), prop);
        if (itMortgage != mortgageableProperties.end()) {
            mortgageableProperties.erase(itMortgage);
        }

        int sellValue = prop->getBuyPrice();
        Street *street = dynamic_cast<Street *>(prop);
        if (street != nullptr) {
            const int buildingCount = street->getBuildingCount();
            if (buildingCount >= static_cast<int>(BuildingLevel::HOTEL)) {
                sellValue += street->getHotelCost() / 2;
            } else if (buildingCount > 0) {
                sellValue += (buildingCount * street->getHouseCost()) / 2;
            }
        }

        debtor->addCash(sellValue);
        debtor->removeProperty(prop);
        prop->setOwner(nullptr);
        prop->setStatusStr("BANK");
        prop->setFestival(1, 0);
        prop->setBuildingCount(0);

        return true;
    }
    return false;
}

bool LiquidationPanel::mortgage(Property *prop) {
    if (debtor == nullptr) {
        return false;
    }
    auto it = find(mortgageableProperties.begin(), mortgageableProperties.end(), prop);

    if (it != mortgageableProperties.end()) {
        mortgageableProperties.erase(it);

        int mortgageValue = prop->getMortgageValue();

        if (prop->getStatusString() == "OWNED") {
            debtor->addCash(mortgageValue);
            prop->setStatusStr("MORTGAGED");
        }

        return true;
    }
    return false;
}

bool LiquidationPanel::isDebtSatisfied() {
    if (debtor == nullptr)
        return false;
    return debtor->getCash() >= debtAmount;
}

int LiquidationPanel::getRemainingDebt() {
    if (debtor == nullptr) {
        return 0;
    }
    return max(0, debtAmount - debtor->getCash());
}
