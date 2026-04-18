#include "core/Board-Tiles/PPHTaxTile.hpp"
#include "core/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
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

  int choice = 0;
  std::cin >> choice;
  if (choice != 0 && choice != 1) {
    choice = 0;
  }

  const int selectedTax = (choice == 0) ? flatTax : perTax;
  const int fallbackTax = (choice == 0) ? perTax : flatTax;

  if (player.canPay(selectedTax)) {
    game.executeTaxPayment(player, selectedTax, true);
    return;
  }

  if (player.canPay(fallbackTax)) {
    game.executeTaxPayment(player, fallbackTax, true);
    return;
  }

  Player *creditor = nullptr;
  game.executeBankruptcy(player, creditor, selectedTax);
}
