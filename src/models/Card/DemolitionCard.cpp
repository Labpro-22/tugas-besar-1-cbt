#include "models/Card/DemolitionCard.hpp"
#include "views/InputHandler.hpp"

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
        throw InternalGameException("DemolitionCard::use menerima konteks yang tidak valid.");
    }

    if (!p->canUseAbility()) {
        throw AbilityAlreadyUsedException();
    }

    std::vector<Property*> targets;
    for (Player& player : gm->getPlayers()) {
        if (&player == p || player.getStatus() == BANKRUPT) {
            continue;
        }

        for (Property* prop : player.getProperties()) {
            if (prop == nullptr) {
                continue;
            }
            targets.push_back(prop);
        }
    }

    if (targets.empty()) {
        throw AbilityTargetException("Tidak ada properti lawan untuk dihancurkan.");
    }

    std::cout << "Pilih properti untuk DemolitionCard:\n";
    for (size_t i = 0; i < targets.size(); ++i) {
        Player* owner = targets[i]->getOwner();
        std::cout << (i + 1) << ". " << targets[i]->getCode()
            << " - " << targets[i]->getName()
            << " (pemilik: " << (owner != nullptr ? owner->getUsername() : "-")
            << ")\n";
    }

    InputHandler input;
    const int choice = input.readChoice(1, static_cast<int>(targets.size()), "Pilih properti untuk DemolitionCard: ");

    Property* target = targets[static_cast<std::size_t>(choice - 1)];
    const std::string targetName = target->getName();
    const std::string targetCode = target->getCode();
    gm->destroyProperty(*p, *target);
    std::cout << "Properti " << targetName
              << " dihancurkan oleh DemolitionCard dan kembali ke Bank.\n";

    gm->addLogEntry(p->getUsername() + " menghancurkan properti " + targetCode);
    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);
}
