#include "ChanceCard.hpp"
#include "../GameManager/Player.hpp"
#include "../GameManager/GameManager.hpp"

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
    if (p == nullptr || gm == nullptr) {
        return;
    }

    switch (chanceType) {
        case ChanceCardType::GO_TO_NEAREST_STATION: {
            const int destination = gm->getNearestStationPosition(p->getPosition());
            gm->movePlayerTo(*p, destination);
            gm->addLogEntry(p->getUsername() + " menuju stasiun terdekat");
            break;
        }
        case ChanceCardType::MOVE_BACK_3: {
            gm->movePlayerTo(*p, p->getPosition() - 3);
            gm->addLogEntry(p->getUsername() + " mundur 3 petak");
            break;
        }
        case ChanceCardType::GO_TO_JAIL: {
            gm->sendPlayerToJail(*p);
            break;
        }
    }
}
