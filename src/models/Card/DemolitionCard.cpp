#include "../GameManager/Player.hpp"
#include "DemolitionCard.hpp"
#include "../Property/Street.hpp"

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
    (void)gm;
    if (p == nullptr) {
        return;
    }

    // Menghancurkan 1 properti lawan
    for (Property* property : p->getProperties()) {
        Street* street = dynamic_cast<Street*>(property);
        if (street != nullptr && street->getBuildingLevel() != BuildingLevel::EMPTY) {
            street->demolish();
            break;
        }
    }

    markAsUsed();
}
