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
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() + " gagal menggunakan TeleportCard (ability sudah dipakai)");
        return;
    }

    int targetTile = 0;
    std::cout << "Pilih tile tujuan teleport (0-39): ";
    std::cin >> targetTile;

    if (targetTile < 0 || targetTile >= 40) {
        std::cout << "Input tidak valid. Teleport dibatalkan.\n";
        return;
    }

    p->setPosition(targetTile);
    gm->addLogEntry(p->getUsername() + " berteleportasi ke tile " + std::to_string(targetTile));
    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
}
