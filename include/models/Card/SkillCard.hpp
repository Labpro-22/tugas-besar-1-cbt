#pragma once
#include "Card.hpp"

class SkillCard : public Card {
    private:
        bool used;
    public:
        // ctor
        SkillCard();

        // ctor with id
        SkillCard(int cardId);

        // dtor
        virtual ~SkillCard();

        // Getter used
        bool isUsed() const;

        // Mark this skill card as used
        void markAsUsed();

        // Use function (pure virtual)
        virtual void use(Player* p, GameManager* gm) = 0;
        virtual int getValue() const override {
            return 0;
        }
};