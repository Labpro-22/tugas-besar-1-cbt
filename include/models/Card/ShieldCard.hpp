#pragma once
#include "SkillCard.hpp"

class ShieldCard : public SkillCard {
    public:
        // ctor
        ShieldCard();

        // ctor with id
        ShieldCard(int cardId);

        // dtor
        ~ShieldCard();

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        void use(Player* p, GameManager* gm) override;
};
