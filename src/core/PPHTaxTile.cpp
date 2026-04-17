#include "core/PPHTaxTile.hpp"
#include <iostream>

PPHTaxTile::PPHTaxTile(const std::string &code, const std::string &name, int pos, int flat, int percentage)
  : TaxTile(code, name, pos, "pph"), pph_flat(flat), pph_percentage(percentage) {}

int PPHTaxTile::calculateTax(Player &player, int tax) {
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
  const int flatTax = getFlatTax();
  const int perTax = calculateTax(player, getPercentage());

  int choice;
  std::cin >> choice;

  switch (choice) {
    case 0:
      if (!player.canPay(flatTax)) {
        if (!player.canPay(perTax)) {
          Player *creditor = nullptr;
          game.executeBankruptcy(player, creditor, flatTax);
          return;
        }
        game.executeTaxPayment(player, perTax, true);
      }
      game.executeTaxPayment(player, flatTax, true);

    case 1:
      if (!player.canPay(perTax)) {
        if (!player.canPay(flatTax)) {
          Player *creditor = nullptr;
          game.executeBankruptcy(player, creditor, perTax);
          return;
        }
        game.executeTaxPayment(player, flatTax, true);
      }
      game.executeTaxPayment(player, perTax, true);
  }
}
