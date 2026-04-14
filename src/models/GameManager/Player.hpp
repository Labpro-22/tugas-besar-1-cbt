#pragma once
#include <iostream>
#include <vector>
#include "../Card/Card.hpp"
#include "../Card/SkillCard.hpp"
using namespace std;

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
        int jailTurns;
        vector<Card> hand;
        vector<Property> properties;
        bool usedAbilityThisTurn;
        bool hasShield;
        int discountPercentage;
        int discountRemainingTurns;
    public :
        Player(std::string username, int startingCash);
        string getUsername() const;
        int getCash() const;
        int getPosition() const;
        PlayerStatus getStatus() const;
        void addCash(int amount);
        void reduceCash(int amount);
        int getTotalWealth() const;
        void addProperty(Property* prop);
        void removeProperty(Property* prop);
        vector<Property*>& getProperties();
        int getPropertyCount() const;
        void addCard(SkillCard* card);
        void removeCard(SkillCard* card);
        vector<SkillCard*>& getHand(); 
        int getCardCount() const;
        bool canUseAbility() const;
        void setUsedAbility();
        void resetTurn();
        void payJailFine();
        bool isJailed() const;
        bool canGetCard() const;
        void setBankrupt();
        void setPosition(int pos);
        void activateShield();
        void deactivateShield();
        bool hasShieldActive() const;
        void applyDiscount(int pct);
        void tickDiscount();
        int getDiscountPercentage() const;
        bool hasDiscount() const;
        bool canPay(int amount) const;
        int getRailroadCount() const;
        int getUtilityCount() const;
};