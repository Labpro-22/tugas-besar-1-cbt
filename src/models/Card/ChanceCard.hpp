#pragma once
#include "ActionCard.hpp"
#include "ChanceCardType.hpp"

class Player;
class GameManager;

class ChanceCard : public ActionCard {
private:
    ChanceCardType chanceType;

public:
    // ctor
    ChanceCard(int cardId, ChanceCardType type);

    // dtor
    ~ChanceCard();

    // Must implement functions
    std::string getDescription() const override;
    std::string getType() const override;
    void execute(Player *p, GameManager *gm) override;
};