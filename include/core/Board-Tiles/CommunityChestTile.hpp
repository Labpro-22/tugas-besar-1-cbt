#ifndef COMMUNITY_CHEST_TILE_HPP
#define COMMUNITY_CHEST_TILE_HPP

#include "CardTile.hpp"
#include "models/Card/CommunityCardType.hpp"

#include <vector>

class CommunityChestTile : public CardTile {
    private:
        static std::vector<CommunityCardType> makeCommunityDeck();
        static CommunityCardType drawCommunityCardType();

    public:
        // Konstruktor dan destruktor
        CommunityChestTile(const std::string &code, const std::string &name, int pos);
        ~CommunityChestTile() override = default;

        // Handler apabila ada player di petak
        void drawCardandExecute(Player &player, GameManager &game) override;
        void onLanded(Player &player, GameManager &game) override;
};

#endif
