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
    (void)p;
    (void)gm;
    markAsUsed();
    // implement effect through GameManager (later)
}
