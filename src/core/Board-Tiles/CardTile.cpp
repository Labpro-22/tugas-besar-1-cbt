#include "../../../include/core/Board-Tiles/CardTile.hpp"

CardTile::CardTile(const std::string &code, const std::string &name, int pos, const std::string &cardType) 
    : Tile(code, name, pos, "card"), card_type(cardType) {}

std::string CardTile::getCardType() const { return card_type; }

void CardTile::drawCardandExecute(Player &player, GameManager &game) {
    (void)player;
    (void)game;
}
