#include "ChanceCard.hpp"

// ctor
ChanceCard::ChanceCard(int cardId, ChanceCardType type) : ActionCard(cardId), chanceType(type) {}

// dtor
ChanceCard::~ChanceCard() {}

// Must implement functions
std::string ChanceCard::getDescription() const {
    switch (chanceType) {
        case ChanceCardType::GO_TO_NEAREST_STATION:
            return "Go to nearest station.";
        case ChanceCardType::MOVE_BACK_3:
            return "Move back 3 tiles.";
        case ChanceCardType::GO_TO_JAIL:
            return "Go directly to jail.";
        default:
            return "Unknown chance card.";
    }
}

std::string ChanceCard::getType() const {
    return "ChanceCard";
}

void ChanceCard::execute(Player* p, GameManager* gm) {
    (void)p;
    (void)gm;
    // implement effect through GameManager (later)
}
