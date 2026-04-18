#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "../Card/Card.hpp"
#include "../Property/Property.hpp"
using namespace std;

class JailManager;

// struct PlayerStatus {
//     enum Kategori {
//         ACTIVE,
//         BANKRUPT,
//         JAILED, 
//     };
// }
enum PlayerStatus {
    ACTIVE,
    BANKRUPT,
    JAILED, 
};
class Player{
    private:
        string username;
        int cash;
        int position;
        PlayerStatus status;
        vector<Card*> hand;
        vector<Property*> properties;
        bool usedAbilityThisTurn;
        bool hasShield;
        int discountPercentage;
        int discountRemainingTurns;
    public :
        Player(string username, int startingCash, int startPosition = 0, PlayerStatus state = ACTIVE, vector<Card*> startHand = {}, vector<Property*> startProperty = {}, int usedAbility = 0, bool shield = false, int discPercent = 0, int discRemain = 0);
        string getUsername();
        int getCash() const;
        int getPosition() const;
        PlayerStatus getStatus() const;
        void setStatus(int State);
        void addCash(int amount);
        void reduceCash(int amount);
        int getTotalWealth() const;
        void addProperty(Property* prop);
        void removeProperty(Property* prop);
        vector<Property*>& getProperties();
        int getPropertyCount() const;
        void addCard(Card* card);
        void removeCard(Card* card);
        vector<Card*>& getHand(); 
        int getCardCount() const;
        bool canUseAbility() const;
        void setUsedAbility();
        void resetTurn();
        void payJailFine(JailManager& jm);
        bool isJailed() const;
        bool canGetCard() const;
        void setBankrupt();
        void setPosition(int pos);
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