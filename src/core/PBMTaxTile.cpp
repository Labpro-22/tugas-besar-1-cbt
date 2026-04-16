#include "core/PBMTaxTile.hpp"

PBMTaxTile::PBMTaxTile(const std::string &code, const std::string &name,
                       int pos, int flat)
    : TaxTile(code, name, pos, "pbm"), pbm_flat(flat) {}

int PBMTaxTile::getFlatTax() const { return pbm_flat; }

void PBMTaxTile::onLanded(Player &player, GameManager &game) {
  // Implementasi fungsi pembayaran pajak
}
