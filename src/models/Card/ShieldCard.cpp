#include "ShieldCard.hpp"
#include "../GameManager/Player.hpp"
#include "../GameManager/GameManager.hpp"

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
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() + " gagal menggunakan ShieldCard (ability sudah dipakai)");
        return;
    }

    p->activateShield();

    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
    gm->addLogEntry(p->getUsername() + " mengaktifkan shield");
}
