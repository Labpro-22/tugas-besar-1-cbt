#include "BankruptcyHandler.hpp"
#include "../Property/Property.hpp"
#include "LiquidationPanel.hpp"
#include "Player.hpp"

BankruptcyHandler::BankruptcyHandler(Player &debtor, Player *creditor, int debt)
    : debtor(debtor), creditor(creditor), debtAmount(debt),
        panel(new LiquidationPanel(&debtor, debt)) {}

BankruptcyHandler::~BankruptcyHandler() { delete panel; }

int BankruptcyHandler::calculateMaxLiquidation() {

    int total = 0;
    total += debtor.getCash();

    vector<Property *> sellList = panel->getSellableProperties();

    for (size_t i = 0; i < sellList.size(); i++) {
        total += sellList[i]->getBuyPrice();
    }
    return total;
}
bool BankruptcyHandler::canCoverDebt() {
    return calculateMaxLiquidation() >= debtAmount;
}
bool BankruptcyHandler::initiateLiquidation() {
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
        debtor.reduceCash(debtAmount);
        return true;
    }
    declareBankrupt();
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
        creditor->addProperty(props[i]);
        props[i]->setOwner(creditor);
    }

    creditor->addCash(debtor.getCash());
    debtor.reduceCash(debtor.getCash());
}
void BankruptcyHandler::repossessProperties() {
    vector<Property *> props = debtor.getProperties();

    for (size_t i = 0; i < props.size(); i++) {
        props[i]->setOwner(nullptr);
    }

    for (size_t i = 0; i < props.size(); i++) {
        debtor.removeProperty(props[i]);
    }
    debtor.reduceCash(debtor.getCash());
}

void BankruptcyHandler::auctionRepossessedProperties() {
    cout << "Properti sitaan masuk ke pelelangan." << endl;
}