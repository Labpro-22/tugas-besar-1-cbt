#pragma once
#include "Card.hpp"

class ActionCard : public Card {
    public:
        // ctor
        ActionCard();

        // custom ctor
        ActionCard(int cardId);

        // dtor
        virtual ~ActionCard();

        // exec functions
        virtual void execute(Player* p, GameManager* gm) = 0;
};