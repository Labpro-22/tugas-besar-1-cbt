#include "Player.hpp"
#include "JailManager.hpp"

Player::Player(string username, int startingCash, PlayerStatus state,
    int startPosition, vector<Card *> startHand,
    vector<Property *> startProperty, int usedAbility, bool shield,
    int discPercent, int discRemain)
    : username(username), cash(startingCash), position(startPosition),
        status(state), hand(startHand), properties(startProperty),
        usedAbilityThisTurn(usedAbility != 0), hasShield(shield),
        discountPercentage(discPercent), discountRemainingTurns(discRemain) {}

string Player::getUsername() { return username; }
int Player::getCash() const { return cash; }
int Player::getPosition() const { return position; }
PlayerStatus Player::getStatus() const { return status; }

void Player::setStatus(int State) { status = static_cast<PlayerStatus>(State); }
void Player::setPosition(int pos) { position = pos; }

void Player::setActive() { status = ACTIVE; }

void Player::addCash(int amount) { cash += amount; }
void Player::reduceCash(int amount) { cash -= amount; }
bool Player::canPay(int amount) const { return cash >= amount; }

int Player::getTotalWealth() const {
    int totalWealth = cash;
    for (Property *prop : properties) {
        totalWealth += prop->getBuyPrice();
    }
    return totalWealth;
}

void Player::addProperty(Property *prop) { properties.push_back(prop); }

void Player::removeProperty(Property *prop) {
    auto it = find(properties.begin(), properties.end(), prop);
    if (it != properties.end()) {
        properties.erase(it);
    }
}

vector<Property *> &Player::getProperties() { return properties; }
int Player::getPropertyCount() const { return properties.size(); }

void Player::addCard(SkillCard *card) { hand.push_back(card); }

void Player::removeCard(SkillCard *card) {
    auto it = std::find(hand.begin(), hand.end(), card);
    if (it != hand.end()) {
        hand.erase(it);
    }
}

vector<Card *> &Player::getHand() { return hand; }

int Player::getCardCount() const { return hand.size(); }

bool Player::canGetCard() const { return hand.size() < 3; }

bool Player::canUseAbility() const { return !usedAbilityThisTurn; }
void Player::setUsedAbility() { usedAbilityThisTurn = true; }

void Player::resetTurn() {
    usedAbilityThisTurn = false;
    tickDiscount();
}

void Player::setBankrupt() { status = BANKRUPT; }

bool Player::isJailed() const { return status == JAILED; }

void Player::payJailFine(JailManager &jm) {
    if (jm.payFine(*this)) {
        status = ACTIVE;
    } else {
        cout << "Kamu belum bisa bayar Fine\n";
    }
}

// Manajemen Shield (Anti-Serangan)
void Player::activateShield() { hasShield = true; }
void Player::deactivateShield() { hasShield = false; }
bool Player::hasShieldActive() const { return hasShield; }

void Player::applyDiscount(int pct, int duration) {
    discountPercentage = pct;
    discountRemainingTurns = (duration > 0) ? duration : 1;
}

void Player::tickDiscount() {
    if (discountRemainingTurns > 0) {
        discountRemainingTurns--;
        if (discountRemainingTurns == 0) {
            discountPercentage = 0;
        }
    }
}

int Player::getDiscountPercentage() const { return discountPercentage; }
bool Player::hasDiscount() const { return discountPercentage > 0; }

int Player::getRailroadCount() const {
    int count = 0;
    for (Property *prop : properties) {
        if (prop->getType() == "Railroad") {
            count++;
        }
    }
    return count;
}

int Player::getUtilityCount() const {
    int count = 0;
    for (Property *prop : properties) {
        if (prop->getType() == "Utility") {
            count++;
        }
    }
    return count;
}