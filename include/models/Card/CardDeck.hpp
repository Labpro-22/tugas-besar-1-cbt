#pragma once
#include "Card.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>

template <class T>
class CardDeck {
    private:
        std::vector<T> deck;
        std::vector<T> discardPile;
    public:
        // ctor
        CardDeck() = default;

        // Draw the top card from the deck
        T draw();

        // Shuffle the active deck
        void shuffle();

        // Move all discarded cards back into the deck, then shuffle
        void reshuffleDiscard();

        // Add a new card to the deck
        void addCard(const T& card);

        // Add a card to the discard pile
        void discardCard(const T& card);

        // Check whether the deck has no active cards
        bool isEmpty() const;
};

template <class T>
T CardDeck<T>::draw() {
    if (deck.empty()) {
        if (!discardPile.empty()) {
            reshuffleDiscard();
        }
    }

    if (deck.empty()) {
        throw std::runtime_error("Cannot draw from an empty deck!");
    }

    T card = deck.back();
    deck.pop_back();
    return card;
}

template <class T>
void CardDeck<T>::shuffle() {
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(deck.begin(), deck.end(), generator);
}

template <class T>
void CardDeck<T>::reshuffleDiscard() {
    deck.insert(deck.end(), discardPile.begin(), discardPile.end());
    discardPile.clear();
    shuffle();
}

template <class T>
void CardDeck<T>::addCard(const T& card) {
    deck.push_back(card);
}

template <class T>
void CardDeck<T>::discardCard(const T& card) {
    discardPile.push_back(card);
}

template <class T>
bool CardDeck<T>::isEmpty() const {
    return deck.empty();
}