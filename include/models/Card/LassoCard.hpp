#pragma once
#include "SkillCard.hpp"
#include "../GameManager/Player.hpp"
#include "../GameManager/GameManager.hpp"
#include <iostream>
#include <vector>

class LassoCard : public SkillCard {
    public:
        // ctor
        LassoCard();

        // ctor with id
        LassoCard(int cardId);

        // dtor
        ~LassoCard();

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        void use(Player* p, GameManager* gm) override;
};
