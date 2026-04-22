#include "../../../include/models/GameManager/BankruptcyHandler.hpp"
#include "../../../include/models/Property/Property.hpp"
#include "../../../include/models/Property/Street.hpp"
#include "../../../include/models/GameManager/LiquidationPanel.hpp"
#include "../../../include/models/GameManager/Player.hpp"
#include <algorithm>

BankruptcyHandler::BankruptcyHandler(Player &debtor, Player *creditor, int debt)
    : debtor(debtor), creditor(creditor), debtAmount(debt),
        panel(new LiquidationPanel(&debtor, debt)) {}

BankruptcyHandler::~BankruptcyHandler() { delete panel; }

int BankruptcyHandler::calculateMaxLiquidation() {

    int total = 0;
    total += debtor.getCash();

    vector<Property *> sellList = panel->getSellableProperties();

    for (size_t i = 0; i < sellList.size(); i++) {
        if (sellList[i] == nullptr) {
            continue;
        }

        int sellValue = sellList[i]->getBuyPrice();
        Street *street = dynamic_cast<Street *>(sellList[i]);
        if (street != nullptr) {
            const int buildingCount = street->getBuildingCount();
            if (buildingCount >= static_cast<int>(BuildingLevel::HOTEL)) {
                sellValue += street->getHotelCost() / 2;
            } else if (buildingCount > 0) {
                sellValue += (buildingCount * street->getHouseCost()) / 2;
            }
        }
        total += sellValue;
    }
    return total;
}
bool BankruptcyHandler::canCoverDebt() {
    return calculateMaxLiquidation() >= debtAmount;
}
bool BankruptcyHandler::initiateLiquidation() {
    if (!canCoverDebt()) {
        return false;
    }

    if (panel->isDebtSatisfied()) {
        return true;
    }

    vector<Property *> sellList = panel->getSellableProperties();

    for (size_t i = 0; i < sellList.size(); i++) {
        if (panel->isDebtSatisfied()) {
            break;
        }
        panel->sellToBank(sellList[i]);
    }

    vector<Property *> mortgageList = panel->getMortgageableProperties();

    for (size_t i = 0; i < mortgageList.size(); i++) {
        if (panel->isDebtSatisfied()) {
            break;
        }
        panel->mortgage(mortgageList[i]);
    }

    if (panel->isDebtSatisfied()) {
        return true;
    }
    return false;
}
void BankruptcyHandler::sellPropertyToBank(Property &prop) {
    panel->sellToBank(&prop);
}
void BankruptcyHandler::mortgageProperty(Property &prop) {
    panel->mortgage(&prop);
}

void BankruptcyHandler::declareBankrupt() {
    if (creditor != nullptr) {
        transferAssets();
    } else {
        repossessProperties();
        auctionRepossessedProperties();
    }

    debtor.setBankrupt();
}

void BankruptcyHandler::transferAssets() {
    vector<Property *> props = debtor.getProperties();

    for (size_t i = 0; i < props.size(); i++) {
        if (props[i] == nullptr) {
            continue;
        }
        creditor->addProperty(props[i]);
        props[i]->setOwner(creditor);
    }

    const int cashToTransfer = max(0, debtor.getCash());
    creditor->addCash(cashToTransfer);
    debtor.reduceCash(debtor.getCash());
    for (Property *prop : props) {
        if (prop != nullptr) {
            debtor.removeProperty(prop);
        }
    }
}
void BankruptcyHandler::repossessProperties() {
    vector<Property *> props = debtor.getProperties();

    for (size_t i = 0; i < props.size(); i++) {
        if (props[i] == nullptr) {
            continue;
        }
        props[i]->setOwner(nullptr);
        props[i]->setStatusStr("BANK");
        props[i]->setFestival(1, 0);
        props[i]->setBuildingCount(0);
    }

    for (size_t i = 0; i < props.size(); i++) {
        if (props[i] != nullptr) {
            debtor.removeProperty(props[i]);
        }
    }
    debtor.reduceCash(debtor.getCash());
}

void BankruptcyHandler::auctionRepossessedProperties() {
    cout << "Properti sitaan masuk ke pelelangan." << endl;
}
