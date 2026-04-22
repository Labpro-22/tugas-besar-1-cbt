#ifndef PBM_TAX_TILE_HPP
#define PBM_TAX_TILE_HPP

#include "TaxTile.hpp"

class PBMTaxTile : public TaxTile {
    private:
        // Atribut
        int pbm_flat;

    public:
        // Konstruktor dan destruktor
        PBMTaxTile(const std::string &code, const std::string &name, int pos, int flat);
        ~PBMTaxTile() = default;

        // Getter
        int getFlatTax() const;

        // Handle kasus pemabayaran pajak di petak
        void onLanded(Player &player, GameManager &game) override;
};

#endif
