#pragma once
#include "SkillCard.hpp"

class TeleportCard : public SkillCard {
    public:
        // ctor
        TeleportCard();

        // ctor with id
        TeleportCard(int cardId);

        // dtor
        ~TeleportCard();

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        void use(Player* p, GameManager* gm) override;
};
