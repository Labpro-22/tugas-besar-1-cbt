#include "core/CardTile.hpp"

CardTile::CardTile(const std::string &code, const std::string &name, int pos,
                   const std::string &cardType)
    : Tile(code, name, pos), card_type(cardType) {
  tile_type = "card";
}

std::string CardTile::getCardType() const { return card_type; }
