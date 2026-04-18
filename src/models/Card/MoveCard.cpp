#include "MoveCard.hpp"
#include "../GameManager/Player.hpp"
#include "../GameManager/GameManager.hpp"

// ctor
MoveCard::MoveCard() : SkillCard(), steps(0) {}

// custom ctor
MoveCard::MoveCard(int cardId, int steps) : SkillCard(cardId), steps(steps) {}

// dtor
MoveCard::~MoveCard() {}

// Getter steps
int MoveCard::getSteps() const {
    return steps;
}

// Must implement functions
std::string MoveCard::getDescription() const {
    return "Move by " + std::to_string(steps) + " steps.";
}

std::string MoveCard::getType() const {
    return "MoveCard";
}

int MoveCard::getValue() const {
    return steps;
}

void MoveCard::use(Player* p, GameManager* gm){
    if (p == nullptr || gm == nullptr) {
        return;
    }

    if (!p->canUseAbility()) {
        gm->addLogEntry(p->getUsername() + " gagal menggunakan MoveCard (ability sudah dipakai)");
        return;
    }

    if (p == &gm->getCurrentPlayer()) {
        gm->moveCurrentPlayer(steps);
    } else {
        gm->movePlayerTo(*p, p->getPosition() + steps);
    }

    markAsUsed();
    p->setUsedAbility();
    p->removeCard(this);

    gm->addLogEntry("MoveCard digunakan sebanyak " + std::to_string(steps) + " langkah");
}