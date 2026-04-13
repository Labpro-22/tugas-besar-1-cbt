#pragma once
#include "Card.hpp"

// Placeholder (wait for others to implement)
class Player;
class GameManager;

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