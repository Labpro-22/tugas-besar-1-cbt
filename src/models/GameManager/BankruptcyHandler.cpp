#include "models/GameManager/BankruptcyHandler.hpp"
#include "models/Property/Property.hpp"
#include "models/Property/Street.hpp"
#include "models/GameManager/Player.hpp"
#include <algorithm>

BankruptcyHandler::BankruptcyHandler(Player &debtor, Player *creditor, int debt)
    : debtor(debtor), creditor(creditor), debtAmount(debt) {
    buildAssetLists();
}

void BankruptcyHandler::buildAssetLists() {
    sellableProperties.clear();
    mortgageableProperties.clear();
    for (Property *prop : debtor.getProperties()) {
        if (prop == nullptr) continue;
        if (prop->getStatusString() == "OWNED") {
            sellableProperties.push_back(prop);
            if (prop->getBuildingCount() == 0) {
                mortgageableProperties.push_back(prop);
            }
        }
    }
}

bool BankruptcyHandler::sellToBank(Property *prop) {
    auto it = find(sellableProperties.begin(), sellableProperties.end(), prop);
    if (it == sellableProperties.end()) return false;

    sellableProperties.erase(it);
    auto itM = find(mortgageableProperties.begin(), mortgageableProperties.end(), prop);
    if (itM != mortgageableProperties.end()) {
        mortgageableProperties.erase(itM);
    }

    int sellValue = prop->getBuyPrice() + prop->getBuildingSellValue();
    debtor.addCash(sellValue);
    debtor.removeProperty(prop);
    prop->setOwner(nullptr);
    prop->setStatusStr("BANK");
    prop->setFestival(1, 0);
    prop->setBuildingCount(0);
    return true;
}

bool BankruptcyHandler::mortgageProperty(Property *prop) {
    auto it = find(mortgageableProperties.begin(), mortgageableProperties.end(), prop);
    if (it == mortgageableProperties.end()) return false;

    mortgageableProperties.erase(it);
    if (prop->getStatusString() == "OWNED") {
        debtor.addCash(prop->getMortgageValue());
        prop->setStatusStr("MORTGAGED");
    }
    return true;
}

bool BankruptcyHandler::isDebtSatisfied() const {
    return debtor.getCash() >= debtAmount;
}

int BankruptcyHandler::calculateMaxLiquidation() {
    int total = debtor.getCash();
    for (Property *prop : sellableProperties) {
        if (prop == nullptr) continue;
        total += prop->getBuyPrice() + prop->getBuildingSellValue();
    }
    return total;
}

bool BankruptcyHandler::canCoverDebt() {
    return calculateMaxLiquidation() >= debtAmount;
}

bool BankruptcyHandler::initiateLiquidation() {
    if (!canCoverDebt()) return false;
    if (isDebtSatisfied()) return true;

    vector<Property *> sellList = sellableProperties;
    for (size_t i = 0; i < sellList.size(); i++) {
        if (isDebtSatisfied()) break;
        sellToBank(sellList[i]);
    }

    vector<Property *> mortgageList = mortgageableProperties;
    for (size_t i = 0; i < mortgageList.size(); i++) {
        if (isDebtSatisfied()) break;
        mortgageProperty(mortgageList[i]);
    }

    return isDebtSatisfied();
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
        if (props[i] == nullptr) continue;
        creditor->addProperty(props[i]);
        props[i]->setOwner(creditor);
    }
    const int cashToTransfer = max(0, debtor.getCash());
    creditor->addCash(cashToTransfer);
    debtor.reduceCash(debtor.getCash());
    for (Property *prop : props) {
        if (prop != nullptr) debtor.removeProperty(prop);
    }
}

void BankruptcyHandler::repossessProperties() {
    vector<Property *> props = debtor.getProperties();
    for (size_t i = 0; i < props.size(); i++) {
        if (props[i] == nullptr) continue;
        props[i]->setOwner(nullptr);
        props[i]->setStatusStr("BANK");
        props[i]->setFestival(1, 0);
        props[i]->setBuildingCount(0);
    }
    for (size_t i = 0; i < props.size(); i++) {
        if (props[i] != nullptr) debtor.removeProperty(props[i]);
    }
    debtor.reduceCash(debtor.getCash());
}

void BankruptcyHandler::auctionRepossessedProperties() {
    cout << "Properti sitaan masuk ke pelelangan." << endl;
}
