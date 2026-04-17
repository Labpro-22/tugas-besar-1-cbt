#include "LassoCard.hpp"

// ctor
LassoCard::LassoCard() : SkillCard() {}

// ctor with id
LassoCard::LassoCard(int cardId) : SkillCard(cardId) {}

// dtor
LassoCard::~LassoCard() {}

// Must implement functions
std::string LassoCard::getDescription() const {
    return "Pulls a target player to your tile.";
}

std::string LassoCard::getType() const {
    return "LassoCard";
}

void LassoCard::use(Player* p, GameManager* gm) {
    (void)p;
    (void)gm;
    markAsUsed();
    // implement effect through GameManager (later)
}
