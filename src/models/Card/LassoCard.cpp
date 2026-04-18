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

std::string LassoCard::getType() const { return "LassoCard"; }

void LassoCard::use(Player *p, GameManager *gm) {
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() +
            " gagal menggunakan LassoCard (ability sudah dipakai)");
        return;
    }

    std::vector<Player *> candidates;
    for (Player &player : gm->getPlayers()) {
        if (&player == p) {
            continue;
        }
        if (player.getStatus() != ACTIVE) {
            continue;
        }
        candidates.push_back(&player);
    }

    if (candidates.empty()) {
        std::cout << "Tidak ada target valid untuk LassoCard.\n";
        return;
    }

    std::cout << "Pilih target LassoCard:\n";
    for (size_t i = 0; i < candidates.size(); ++i) {
        std::cout << i << ". " << candidates[i]->getUsername() << " (tile "
            << candidates[i]->getPosition() << ")\n";
    }

    int choice = -1;
    std::cin >> choice;
    if (choice < 0 || choice >= static_cast<int>(candidates.size())) {
        std::cout << "Pilihan target tidak valid.\n";
        return;
    }

    Player *target = candidates[choice];
    target->setPosition(p->getPosition());
    gm->addLogEntry(p->getUsername() + " menarik " + target->getUsername() +
        " dengan LassoCard");
    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
}
