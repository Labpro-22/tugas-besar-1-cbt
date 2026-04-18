#pragma once
#include <string>

// Placeholder (wait for others to implement)
class Player;
class GameManager;

class Card {
    private:
        static int nextCardId;
        int cardId;
    public:
        // ctor with auto-increment id
        Card();

        // ctor with id
        Card(int cardId);

        // dtor
        virtual ~Card();

        // Getter Functions
        int getCardId() const;

        // Getter Functions (Pure Virtual)
        virtual std::string getDescription() const = 0;
        virtual std::string getType() const = 0;
};