#include "models/Card/Card.hpp"

// static var
int Card::nextCardId = 1;

// ctor
Card::Card() : cardId(nextCardId++) {}

// ctor with id
Card::Card(int cardId) : cardId(cardId) {
    if (cardId >= nextCardId) {
        nextCardId = cardId + 1;
    }
}

// dtor
Card::~Card(){}

// getter
int Card::getCardId() const{
    return cardId;
}

std::string Card::getDisplayLabel() const {
    std::string label = getType();
    if (getValue() != 0) {
        label += " (" + std::to_string(getValue()) + ")";
    }
    return label;
}