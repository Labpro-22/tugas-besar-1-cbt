#ifndef TAX_TILE_HPP
#define TAX_TILE_HPP

#include "core/Board-Tiles/Tile.hpp"
#include <string>

class TaxTile : public Tile {
protected:
  // Atribut
  std::string tax_type;

public:
  // Konstruktor dan destruktor
  TaxTile(const std::string &code, const std::string &name, int pos, const std::string &taxType);
  ~TaxTile() = default;

  // Method petak bertipe pajak
  virtual int calculateTax(Player &player, int tax);
  virtual bool hasChoice() const;
};

#endif
