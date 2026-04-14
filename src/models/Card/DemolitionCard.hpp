#pragma once
#include "SkillCard.hpp"

class DemolitionCard : public SkillCard {
    public:
        // ctor
        DemolitionCard();

        // ctor with id
        DemolitionCard(int cardId);

        // dtor
        ~DemolitionCard();

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        void use(Player* p, GameManager* gm) override;
};
