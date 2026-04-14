#include "MoveCard.hpp"

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

void MoveCard::use(Player* p, GameManager* gm){
    (void)p;
    (void)gm;
    markAsUsed();
    // implement effect through GameManager (later)
}