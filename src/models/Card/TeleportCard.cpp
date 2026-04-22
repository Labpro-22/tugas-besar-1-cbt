#include "models/Card/TeleportCard.hpp"
#include "models/GameManager/GameManager.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "views/InputHandler.hpp"
#include "exception/NimonspoliExceptions.hpp"

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
    throw InternalGameException("TeleportCard::use menerima konteks yang tidak valid.");
  }

  if (!p->canUseAbility()) {
    throw AbilityAlreadyUsedException();
  }

  int boardSize = gm->getBoardSize();
  if (boardSize <= 0) {
    throw InternalGameException("TeleportCard digunakan saat ukuran board tidak valid.");
  }

  gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU",
                      "TeleportCard - pilih nomor petak tujuan (0-" +
                          std::to_string(boardSize - 1) + ").");
  gm->pushSnapshot();
  InputHandler input;
  const int targetTile = input.readChoice(0, boardSize - 1, "Pilih tile tujuan teleport (0-" + std::to_string(boardSize - 1) + "): ");

  gm->movePlayerTo(*p, targetTile, true);
  Tile &tile = gm->getBoard().getTile(p->getPosition());
  std::cout << "TeleportCard diaktifkan! Bidak dipindahkan ke "
            << tile.getName() << ".\n";

  markAsUsed();
  p->setUsedAbility();
  p->removeCard(this);

  gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU",
                      "TeleportCard: Berteleportasi ke " + tile.getName());
  tile.onLanded(*p, *gm);
}
