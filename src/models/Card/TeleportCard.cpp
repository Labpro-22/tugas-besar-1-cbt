#include "models/Card/TeleportCard.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "views/InputHandler.hpp"

// ctor
TeleportCard::TeleportCard() : SkillCard() {}

// ctor with id
TeleportCard::TeleportCard(int cardId) : SkillCard(cardId) {}

// dtor
TeleportCard::~TeleportCard() {}

// Must implement functions
std::string TeleportCard::getDescription() const {
  return "Teleport to a chosen tile.";
}

std::string TeleportCard::getType() const { return "TeleportCard"; }

void TeleportCard::use(Player *p, GameManager *gm) {
  if (p == nullptr || gm == nullptr) {
    return;
  }

  if (!p->canUseAbility()) {
    gm->addLogEntry(p->getUsername() +
                    " gagal menggunakan TeleportCard (ability sudah dipakai)");
    return;
  }

  int boardSize = gm->getBoardSize();
  if (boardSize <= 0) {
    gm->addLogEntry("TeleportCard gagal: ukuran board tidak valid");
    return;
  }

  InputHandler input;
  const int targetTile = input.readChoice(0, boardSize - 1, "Pilih tile tujuan teleport (0-" + std::to_string(boardSize - 1) + "): ");

  gm->movePlayerTo(*p, targetTile, true);
  Tile &tile = gm->getBoard().getTile(p->getPosition());
  std::cout << "TeleportCard diaktifkan! Bidak dipindahkan ke "
            << tile.getName() << ".\n";

  markAsUsed();
  p->setUsedAbility();
  p->removeCard(this);

  gm->addLogEntry(p->getUsername() + " berteleportasi ke tile " + std::to_string(targetTile));
  tile.onLanded(*p, *gm);
}
