#include "core/PPHTaxTile.hpp"

PPHTaxTile::PPHTaxTile(const std::string &code, const std::string &name,
                       int pos, int flat, int percentage)
    : TaxTile(code, name, pos, "pph"), pph_flat(flat),
      pph_percentage(percentage) {}

int PPHTaxTile::calculateTax(Player &player, int tax) {
  // Implementasi kalkulasi besar pajak yang harus dibayarkan
}

bool PPHTaxTile::hasChoice() const { return true; }

int PPHTaxTile::getFlatTax() const { return pph_flat; }

int PPHTaxTile::getPercentage() const { return pph_percentage; }

void PPHTaxTile::onLanded(Player &player, GameManager &game) {
  // Implementasi fungsi pembayaran pajak
}
