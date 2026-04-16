#include "misc-config.hpp"

MiscConfig::MiscConfig() : maxTurn(0), initialBalance(0) {};
int MiscConfig::getMaxTurn() const {
    return maxTurn;
}
int MiscConfig::getInitialBalance() const {
    return initialBalance;
}
void MiscConfig::setMiscAmount(int maxTurn, int initialBalance) {
    this->maxTurn = maxTurn;
    this->initialBalance = initialBalance;
}