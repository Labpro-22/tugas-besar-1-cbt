#pragma once
#include "ActionCard.hpp"
#include "ChanceCardType.hpp"
#include "Player.hpp"
#include "GameManager.hpp"

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
    int getValue() const override { return 0; }
};
