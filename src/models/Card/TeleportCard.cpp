#include "../GameManager/Player.hpp"
#include "TeleportCard.hpp"

// ctor
TeleportCard::TeleportCard() : SkillCard() {}

// ctor with id
TeleportCard::TeleportCard(int cardId) : SkillCard(cardId) {}

// dtor
TeleportCard::~TeleportCard() {}

// Must implement functions
std::string TeleportCard::getDescription() const {
    return "Teleport to a chosen tile.";
}

std::string TeleportCard::getType() const {
    return "TeleportCard";
}

void TeleportCard::use(Player* p, GameManager* gm) {
    (void)gm;
    if (p == nullptr) {
        return;
    }

    // default
    p->setPosition(0);
    markAsUsed();
}
