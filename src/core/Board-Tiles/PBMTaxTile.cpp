#include "core/Board-Tiles/PBMTaxTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "exception/NimonspoliExceptions.hpp"

PBMTaxTile::PBMTaxTile(const std::string &code, const std::string &name, int pos, int flat)
    : TaxTile(code, name, pos, "pbm"), pbm_flat(flat) {
    if (flat < 0) {
        throw InvalidConfigurationFormatException("tax_config", "PBM flat tidak boleh negatif.");
    }
}

int PBMTaxTile::getFlatTax() const { return pbm_flat; }

void PBMTaxTile::onLanded(Player &player, GameManager &game) {
    try {
        const int finalTax = calculateTax(player, getFlatTax());
        logTileEvent(game, player, "PAJAK",
                     "Mendarat di " + getName() + ". Pajak PBB sebesar M" +
                         std::to_string(finalTax) + " langsung dipotong.");
        game.executeTaxPayment(player, finalTax, true);
    } catch (const NimonspoliException &) {
        throw;
    } catch (const std::exception &e) {
        throw InternalGameException(std::string("PBMTaxTile::onLanded: ") + e.what());
    }
}
