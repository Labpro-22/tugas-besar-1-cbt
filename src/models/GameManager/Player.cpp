#include "models/GameManager/Player.hpp"

#include <stdexcept>

Player::Player(string username, int startingCash, PlayerStatus state,
    int startPosition, vector<Card *> startHand,
    vector<Property *> startProperty, int usedAbility, bool shield,
    int discPercent, int discRemain)
    : username(username), cash(startingCash), position(startPosition),
        status(state), hand(startHand), properties(startProperty),
        usedAbilityThisTurn(usedAbility != 0), hasShield(shield),
        discountPercentage(discPercent), discountRemainingTurns(discRemain) {}

Player::Player(const Player &other)    : username(other.username), cash(other.cash), position(other.position),
      status(other.status), hand(other.hand), properties(other.properties),
      usedAbilityThisTurn(other.usedAbilityThisTurn), hasShield(other.hasShield),  
      discountPercentage(other.discountPercentage), discountRemainingTurns(other.discountRemainingTurns) {}
Player::Player() : username(""), cash(0), position(0), status(ACTIVE), hand({}), properties({}), 
                    usedAbilityThisTurn(false), hasShield(false), discountPercentage(0), discountRemainingTurns(0) {}
string Player::getUsername() const { return username; }
int Player::getCash() const { return cash; }
int Player::getPosition() const { return position; }
PlayerStatus Player::getStatus() const { return status; }

void Player::setStatus(int State) { status = static_cast<PlayerStatus>(State); }
void Player::setPosition(int pos) { position = pos; }

void Player::setActive() { status = ACTIVE; }

void Player::addCash(int amount) { *this += amount; }
void Player::reduceCash(int amount) { *this -= amount; }
bool Player::canPay(int amount) const { return cash >= amount; }

Player& Player::operator+=(int amount) {
    cash += amount;
    return *this;
}

Player& Player::operator-=(int amount) {
    cash -= amount;
    return *this;
}


bool Player::operator<(const Player& other) const {
    return getTotalWealth() < other.getTotalWealth();
}

bool Player::operator>(const Player& other) const {
    return other < *this;
}

void Player::ensureCanPay(int amount) const {
    if (!canPay(amount)) {
        throw std::runtime_error("Uang " + username + " tidak cukup. Butuh M" +
                                 std::to_string(amount) + ", tersedia M" +
                                 std::to_string(cash) + ".");
    }
}

int Player::getTotalWealth() const {
    int totalWealth = cash;
    for (Property *prop : properties) {
        if (prop == nullptr) {
            continue;
        }
        totalWealth += prop->getBuyPrice();
        totalWealth += prop->getBuildingInvestmentValue();
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
const vector<Property *> &Player::getProperties() const { return properties; }
int Player::getPropertyCount() const { return properties.size(); }

void Player::addCard(SkillCard *card) {
    if (hand.size() >= 4) {
        throw std::runtime_error(
            "Slot kartu skill penuh. Maksimal 3 kartu, kartu ke-4 wajib dibuang.");
    }
    hand.push_back(card);
}

void Player::removeCard(SkillCard *card) {
    auto it = std::find(hand.begin(), hand.end(), card);
    if (it != hand.end()) {
        hand.erase(it);
    }
}

vector<Card *> &Player::getHand() { return hand; }
const vector<Card *> &Player::getHand() const { return hand; }

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
