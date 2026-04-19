#include "core/Board-Tiles/TaxTile.hpp"

TaxTile::TaxTile(const std::string &code, const std::string &name, int pos, const std::string &taxType) 
    : Tile(code, name, pos, "tax"), tax_type(taxType) {}

int TaxTile::calculateTax(Player &, int tax) { return tax; }

bool TaxTile::hasChoice() const { return false; }
