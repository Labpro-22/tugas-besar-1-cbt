#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Player.hpp"
using namespace std;


class BankruptcyHandler {
private:
    Player& debtor;
    Player* creditor;
    int debtAmount;

public:
    BankruptcyHandler(Player& debtor, Player* creditor, int debt);

    int calculateMaxLiquidation();
    bool canCoverDebt();
    bool initiateLiquidation();
    void sellPropertyToBank(Property& prop);
    void mortgageProperty(Property& prop);
    void declareBankrupt();
    void transferAssets();
    void repossessProperties();
    void auctionRepossessedProperties();
};