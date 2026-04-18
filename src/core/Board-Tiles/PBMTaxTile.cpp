#include "core/Board-Tiles/PBMTaxTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

PBMTaxTile::PBMTaxTile(const std::string &code, const std::string &name, int pos, int flat)
    : TaxTile(code, name, pos, "pbm"), pbm_flat(flat) {}

int PBMTaxTile::getFlatTax() const { return pbm_flat; }

void PBMTaxTile::onLanded(Player &player, GameManager &game) {
    const int finalTax = calculateTax(player, getFlatTax());

    if (!player.canPay(finalTax)) {
        Player *creditor = nullptr;
        game.executeBankruptcy(player, creditor, finalTax);
        return;
    }

    game.executeTaxPayment(player, finalTax, true);
}
