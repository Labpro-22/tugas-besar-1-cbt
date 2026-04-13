#include "Card.hpp"

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