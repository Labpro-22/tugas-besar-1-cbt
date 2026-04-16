#include "core/TaxTile.hpp"

TaxTile::TaxTile(const std::string &code, const std::string &name, int pos,
                 const std::string &taxType)
    : Tile(code, name, pos), tax_type(taxType) {
  tile_type = "tax";
}

int TaxTile::calculateTax(Player &player, int tax) { return tax; }

bool TaxTile::hasChoice() const { return false; }
