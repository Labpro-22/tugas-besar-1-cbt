#include "views/StatusView.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "exception/NimonspoliExceptions.hpp"
#include "models/Card/Card.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"


void StatusView::showDiceResult(int d1, int d2,
                                const std::string &destination) const {
  std::cout << "Hasil: " << d1 << " + " << d2 << " = " << (d1 + d2) << '\n';
  std::cout << "Bidak mendarat di: " << destination << ".\n";
}

void StatusView::showRentPayment(Player *player, Player *owner, int amount,
                                 Property *property) const {
  if (player == nullptr || owner == nullptr) {
    throw InternalGameException("Informasi pembayaran sewa tidak lengkap.");
  }

  std::cout << player->getUsername() << " membayar sewa sebesar M" << amount
            << " kepada " << owner->getUsername();

  if (property != nullptr) {
    std::cout << " untuk properti " << property->getCode();
  }

  std::cout << ".\n";
}

void StatusView::showTaxPayment(Player *player, int amount) const {
  if (player == nullptr) {
    throw InternalGameException("Informasi pembayaran pajak tidak lengkap.");
  }

  std::cout << player->getUsername() << " membayar pajak sebesar M" << amount
            << ".\n";
}

void StatusView::showFestivalEffect(Property *property, int multiplier,
                                    int duration) const {
  if (property == nullptr) {
    throw InternalGameException("Efek festival tidak dapat ditampilkan.");
  }

  std::cout << "Efek festival aktif pada " << property->getCode()
            << "! Multiplier sewa x" << multiplier << ", durasi " << duration
            << " giliran.\n";
}

void StatusView::showCardUsage(Player *player, Card *card) const {
  if (player == nullptr) {
    throw InternalGameException("Informasi penggunaan kartu tidak lengkap.");
  }

  std::cout << player->getUsername() << " menggunakan kartu ";

  if (card != nullptr) {
    std::cout << card->getType();
  } else {
    std::cout << "[KARTU_TIDAK_DIKETAHUI]";
  }

  std::cout << ".\n";
}

void StatusView::showBankruptcy(Player *debtor, Player *creditor) const {
  if (debtor == nullptr) {
    throw InternalGameException("Informasi kebangkrutan tidak lengkap.");
  }

  std::cout << debtor->getUsername() << " dinyatakan BANGKRUT! ";

  if (creditor != nullptr) {
    std::cout << "Kreditor: " << creditor->getUsername();
  } else {
    std::cout << "Kreditor: Bank";
  }

  std::cout << ".\n";
}

void StatusView::showVictory(const std::vector<Player *> &winners) const {
  std::cout << "Pemenang permainan:\n";

  if (winners.empty()) {
    std::cout << "Tidak ada pemenang.\n";
    return;
  }

  for (Player *winner : winners) {
    if (winner != nullptr) {
      std::cout << "- " << winner->getUsername() << '\n';
    }
  }
}
