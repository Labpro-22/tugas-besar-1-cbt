#ifndef CHANCE_TILE_HPP
#define CHANCE_TILE_HPP

#include "CardTile.hpp"
#include "models/Card/ChanceCardType.hpp"

#include <vector>

class ChanceTile : public CardTile {
    private:
        static std::vector<ChanceCardType> makeChanceDeck();
        static ChanceCardType drawChanceCardType();

    public:
        // Konstruktor dan destruktor
        ChanceTile(const std::string &code, const std::string &name, int pos);
        ~ChanceTile() = default;

        // Menghandle kasus ketika ada player di petak
        void drawCardandExecute(Player &player, GameManager &game) override;
        void onLanded(Player &player, GameManager &game) override;
};

#endif
