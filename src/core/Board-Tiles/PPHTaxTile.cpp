#include "core/Board-Tiles/PPHTaxTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "views/InputHandler.hpp"
#include "exception/NimonspoliExceptions.hpp"
#include <string>

PPHTaxTile::PPHTaxTile(const std::string &code, const std::string &name, int pos, int flat, int percentage)
    : TaxTile(code, name, pos, "pph"), pph_flat(flat), pph_percentage(percentage) {
    if (flat < 0) {
        throw InvalidConfigurationFormatException("tax_config", "PPH flat tidak boleh negatif.");
    }
    if (percentage < 0 || percentage > 100) {
        throw InvalidConfigurationFormatException("tax_config", "PPH percentage harus 0..100.");
    }
}

int PPHTaxTile::calculateTax(Player &player, int tax) {
    if (tax < 0) {
        throw InvalidInputException("Persentase pajak tidak valid.", "INVALID_TAX_PERCENTAGE");
    }
    if (tax == getFlatTax()) {
        return tax;
    }

    const int totalWealth = player.getTotalWealth();
    return (totalWealth * tax) / 100;
}

bool PPHTaxTile::hasChoice() const { return true; }

int PPHTaxTile::getFlatTax() const { return pph_flat; }

int PPHTaxTile::getPercentage() const { return pph_percentage; }

void PPHTaxTile::onLanded(Player &player, GameManager &game) {
    try {
        const int flatTax = getFlatTax();
        const int perTax = calculateTax(player, getPercentage());

        logTileEvent(game, player, "PAJAK",
                     "Mendarat di Pajak Penghasilan. Opsi: flat M" +
                         std::to_string(flatTax) + " atau " +
                         std::to_string(getPercentage()) +
                         "% dari total kekayaan (M" + std::to_string(perTax) +
                         "). Uang kamu: M" + std::to_string(player.getCash()) + ".");
        game.pushSnapshot();
        InputHandler input;
        const int choice = input.readChoice(1, 2, "Pilihan (1/2): ");

        const int selectedTax = (choice == 1) ? flatTax : perTax;
        if (selectedTax < 0) {
            throw TaxPaymentException(player.getUsername(), selectedTax);
        }
        logTileEvent(game, player, "PAJAK",
                     "Memilih opsi pajak " +
                         std::string(choice == 1 ? "flat" : "persentase") +
                         " sebesar M" + std::to_string(selectedTax) + ".");
        game.executeTaxPayment(player, selectedTax, true);
    } catch (const NimonspoliException &) {
        throw;
    } catch (const std::exception &e) {
        throw InternalGameException(std::string("PPHTaxTile::onLanded: ") + e.what());
    }
}
