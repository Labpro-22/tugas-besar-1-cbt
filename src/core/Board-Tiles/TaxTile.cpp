#include "core/Board-Tiles/TaxTile.hpp"
#include "exception/NimonspoliExceptions.hpp"

TaxTile::TaxTile(const std::string &code, const std::string &name, int pos, const std::string &taxType) 
    : Tile(code, name, pos, "tax"), tax_type(taxType) {}

int TaxTile::calculateTax(Player &, int tax) {
    if (tax < 0) {
        throw InvalidInputException("Nilai pajak tidak boleh negatif.",
                                    "INVALID_TAX_VALUE");
    }
    return tax;
}

bool TaxTile::hasChoice() const { return false; }
