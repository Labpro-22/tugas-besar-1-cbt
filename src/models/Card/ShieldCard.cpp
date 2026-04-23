#include "models/Card/ShieldCard.hpp"
#include "models/GameManager/GameManager.hpp"

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
        gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU",
                            "Gagal menggunakan ShieldCard (ability sudah dipakai)");
        return;
    }

    p->activateShield();
    std::cout << "ShieldCard diaktifkan! Kamu kebal terhadap tagihan atau sanksi" " selama giliran ini.\n";

    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
    gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "SHIELD", "ShieldCard aktif - kebal tagihan giliran ini");
}
