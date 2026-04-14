#include "ShieldCard.hpp"

// ctor
ShieldCard::ShieldCard() : SkillCard() {}

// ctor with id
ShieldCard::ShieldCard(int cardId) : SkillCard(cardId) {}

// dtor
ShieldCard::~ShieldCard() {}

// Must implement functions
std::string ShieldCard::getDescription() const {
    return "Protects the player from one negative effect.";
}

std::string ShieldCard::getType() const {
    return "ShieldCard";
}

void ShieldCard::use(Player* p, GameManager* gm) {
    (void)p;
    (void)gm;
    markAsUsed();
    // implement effect through GameManager (later)
}
