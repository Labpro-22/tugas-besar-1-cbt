#include "SkillCard.hpp"

// ctor
SkillCard::SkillCard() : Card(), used(false) {}

// ctor with id
SkillCard::SkillCard(int cardId) : Card(cardId), used(false) {}

// dtor
SkillCard::~SkillCard() {}

// Getter used
bool SkillCard::isUsed() const {
    return used;
}

// Mark as used card
void SkillCard::markAsUsed() {
    used = true;
}
