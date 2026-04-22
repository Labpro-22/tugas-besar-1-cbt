#pragma once
#include "../Card/Card.hpp"
#include "../Card/SkillCard.hpp"
#include "../Property/Property.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum PlayerStatus {
    ACTIVE,
    BANKRUPT,
    JAILED,
};

class Player {
private:
    string username;
    int cash;
    int position;
    PlayerStatus status;
    vector<Card *> hand;
    vector<Property *> properties;
    bool usedAbilityThisTurn;
    bool hasShield;
    int discountPercentage;
    int discountRemainingTurns;

public:
    Player(string username, int startingCash, PlayerStatus state = ACTIVE,
        int startPosition = 0, vector<Card *> startHand = {},
        vector<Property *> startProperty = {}, int usedAbility = 0,
        bool shield = false, int discPercent = 0, int discRemain = 0);
    Player(const Player &other);
    Player();
    string getUsername() const;
    int getCash() const;
    int getPosition() const;
    PlayerStatus getStatus() const;
    void setStatus(int State);
    void addCash(int amount);
    void reduceCash(int amount);
    Player& operator+=(int amount);
    Player& operator-=(int amount);
    bool operator<(const Player& other) const;
    bool operator>(const Player& other) const;
    void ensureCanPay(int amount) const;
    int getTotalWealth() const;
    void addProperty(Property *prop);
    void removeProperty(Property *prop);
    vector<Property *> &getProperties();
    const vector<Property *> &getProperties() const;
    int getPropertyCount() const;
    void addCard(SkillCard *card);
    void removeCard(SkillCard *card);
    vector<Card *> &getHand();
    const vector<Card *> &getHand() const;
    int getCardCount() const;
    bool canUseAbility() const;
    void setUsedAbility();
    void resetTurn();
    bool isJailed() const;
    bool canGetCard() const;
    void setBankrupt();
    void setPosition(int pos);
    void setActive();
    void activateShield();
    void deactivateShield();
    bool hasShieldActive() const;
    void applyDiscount(int pct, int duration = 1);
    void tickDiscount();
    int getDiscountPercentage() const;
    bool hasDiscount() const;
    bool canPay(int amount) const;
    int getRailroadCount() const;
    int getUtilityCount() const;
};

