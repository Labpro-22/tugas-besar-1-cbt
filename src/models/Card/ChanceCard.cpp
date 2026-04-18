#include "ChanceCard.hpp"
#include "../GameManager/GameManager.hpp"
#include "../GameManager/Player.hpp"

// ctor
ChanceCard::ChanceCard(int cardId, ChanceCardType type)
    : ActionCard(cardId), chanceType(type) {}

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

std::string ChanceCard::getType() const { return "ChanceCard"; }

void ChanceCard::execute(Player *p, GameManager *gm) {
    if (p == nullptr || gm == nullptr) {
        return;
    }

    switch (chanceType) {
    case ChanceCardType::GO_TO_NEAREST_STATION: {
        // hard coded
        const int stations[4] = {5, 15, 25, 35};
        const int currentPos = p->getPosition();
        int destination = stations[0];
        for (int i = 0; i < 4; ++i) {
            if (stations[i] > currentPos) {
                destination = stations[i];
                break;
            }
        }

        int oldPos = p->getPosition();
        int newPos = destination % 40;
        if (newPos < 0)
            newPos += 40;
        p->setPosition(newPos);
        if (newPos < oldPos) {
            p->addCash(200);
        }
        gm->addLogEntry(p->getUsername() + " menuju stasiun terdekat");
        break;
    }
    case ChanceCardType::MOVE_BACK_3: {
        int newPos = (p->getPosition() - 3) % 40;
        if (newPos < 0)
            newPos += 40;
        p->setPosition(newPos);
        gm->addLogEntry(p->getUsername() + " mundur 3 petak");
        break;
    }
    case ChanceCardType::GO_TO_JAIL: {
        p->setPosition(10);
        p->setStatus(JAILED);
        gm->addLogEntry(p->getUsername() + " dikirim ke penjara");
        break;
    }
    }
}
