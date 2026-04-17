#include "../GameManager/Player.hpp"
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
    (void)gm;
    if (p == nullptr) {
        return;
    }

    // asumsi: move back 3 merugikan
    if ((chanceType == ChanceCardType::MOVE_BACK_3 || chanceType == ChanceCardType::GO_TO_JAIL) && p->hasShieldActive()) {
        p->deactivateShield();
        return;
    }

    // hardcoded
    constexpr int boardSize = 40;
    constexpr int jailPosition = 10;
    constexpr int stationPositions[] = {5, 15, 25, 35};

    switch (chanceType) {
        case ChanceCardType::GO_TO_NEAREST_STATION: {
            int currentPosition = p->getPosition();
            int targetPosition = stationPositions[0];
            int bestDistance = boardSize;

            for (int stationPosition : stationPositions) {
                int distance = (stationPosition - currentPosition + boardSize) % boardSize;
                if (distance == 0) {
                    targetPosition = stationPosition;
                    break;
                }
                if (distance < bestDistance) {
                    bestDistance = distance;
                    targetPosition = stationPosition;
                }
            }

            p->setPosition(targetPosition);
            // melewati start
            if (targetPosition < currentPosition) {
                p->addCash(200);
            }
            break;
        }
        case ChanceCardType::MOVE_BACK_3: {
            int newPosition = (p->getPosition() - 3) % boardSize;
            
            // memastikan tidak negatif
            if (newPosition < 0) {
                newPosition += boardSize;
            }
            p->setPosition(newPosition);
            break;
        }
        case ChanceCardType::GO_TO_JAIL:
            p->setPosition(jailPosition);
            p->setStatus(JAILED);
            break;
    }
}
