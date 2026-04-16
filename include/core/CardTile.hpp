#ifndef CARD_TILE_HPP
#define CARD_TILE_HPP

#include "core/Tile.hpp"
#include <string>

// Deklarasi di awal
template <typename T> class CardDeck;
class Card;

class CardTile : public Tile {
protected:
  // Atribut
  std::string card_type;

public:
  // Konstruktor dan Destruktor
  CardTile(const std::string &code, const std::string &name, int pos,
           const std::string &cardType);
  ~CardTile() = default;

  // Getter
  std::string getCardType() const;

  // Method untuk mengambil kartu dari dek
  virtual void drawCardandExecute(Player &player, GameManager &game);
};

#endif // CARD_TILE_HPP
