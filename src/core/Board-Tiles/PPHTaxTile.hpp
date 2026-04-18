#ifndef PPH_TAX_TILE_HPP
#define PPH_TAX_TILE_HPP

#include "core/Board-Tiles/TaxTile.hpp"

class PPHTaxTile : public TaxTile {
  private:
    // Atribut
    int pph_flat;
    int pph_percentage;

  public:
    // Konstruktor dan destruktor
    PPHTaxTile(const std::string &code, const std::string &name, int pos, int flat, int percentage);
    ~PPHTaxTile() = default;

    // Menghitung besar pajak
    int calculateTax(Player &player, int tax) override;
    bool hasChoice() const override;

    // Getter
    int getFlatTax() const;
    int getPercentage() const;

    // Handler pembayaran pajak
    void onLanded(Player &player, GameManager &game) override;
};

#endif
