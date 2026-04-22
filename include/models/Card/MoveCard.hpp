#pragma once
#include "SkillCard.hpp"
#include "../GameManager/Player.hpp"
#include "../GameManager/GameManager.hpp"

class MoveCard : public SkillCard {
    private:
        int steps;
    public:
        // ctor
        MoveCard();

        // ctor with id and steps
        MoveCard(int cardId, int steps);

        // dtor
        ~MoveCard();

        // Getter functions
        int getSteps() const;

        // Must implement functions
        std::string getDescription() const override;
        std::string getType() const override;
        int getValue() const override;

        // Use functions
        void use(Player* p, GameManager* gm) override;
};