#pragma once
#include "SkillCard.hpp"
#include <algorithm>
#include "../GameManager/Player.hpp"
#include "../GameManager/GameManager.hpp"

class DiscountCard : public SkillCard {
    private:
        int discountPercent;
        int remainingDuration;
    public:
        // ctor
        DiscountCard();

        // ctor with id and discount parameters
        DiscountCard(int cardId, int discountPercent, int remainingDuration);

        // dtor
        ~DiscountCard();

        // Getter functions
        int getDiscountPercent() const;
        int getRemainingDuration() const;

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        int getValue() const override;
        int getDuration() const override;

        // apply discount
        int applyDiscount(int amount);
        
        // Use functions
        void use(Player* p, GameManager* gm) override;
};
