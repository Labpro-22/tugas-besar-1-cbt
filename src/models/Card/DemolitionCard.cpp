#include "DemolitionCard.hpp"

// ctor
DemolitionCard::DemolitionCard() : SkillCard() {}

// ctor with id
DemolitionCard::DemolitionCard(int cardId) : SkillCard(cardId) {}

// dtor
DemolitionCard::~DemolitionCard() {}

// Must implement functions
std::string DemolitionCard::getDescription() const {
    return "Destroy one building on a selected tile.";
}

std::string DemolitionCard::getType() const {
    return "DemolitionCard";
}

void DemolitionCard::use(Player* p, GameManager* gm) {
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() + " gagal menggunakan DemolitionCard (ability sudah dipakai)");
        return;
    }

    std::vector<Property*> targets;
    for (Player& player : gm->getPlayers()) {
        for (Property* prop : player.getProperties()) {
            if (prop == nullptr) {
                continue;
            }
            if (prop->getBuildingCount() <= 0) {
                continue;
            }
            targets.push_back(prop);
        }
    }

    if (targets.empty()) {
        std::cout << "Tidak ada properti dengan bangunan untuk dihancurkan.\n";
        return;
    }

    std::cout << "Pilih properti untuk DemolitionCard:\n";
    for (size_t i = 0; i < targets.size(); ++i) {
        std::cout << i << ". " << targets[i]->getCode()
            << " (bangunan: " << targets[i]->getBuildingCount() << ")\n";
    }

    int choice = -1;
    std::cin >> choice;
    if (choice < 0 || choice >= static_cast<int>(targets.size())) {
        std::cout << "Pilihan properti tidak valid.\n";
        return;
    }

    Property* target = targets[choice];
    target->demolish();
    gm->addLogEntry(p->getUsername() + " menghancurkan bangunan di " + target->getCode());
    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
}
