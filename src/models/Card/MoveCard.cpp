#include "models/Card/MoveCard.hpp"
#include "models/GameManager/GameManager.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include <iostream>
#include "exception/NimonspoliExceptions.hpp"

// ctor
MoveCard::MoveCard() : SkillCard(), steps(0) {}

// custom ctor
MoveCard::MoveCard(int cardId, int steps) : SkillCard(cardId), steps(steps) {}

// dtor
MoveCard::~MoveCard() {}

// Getter steps
int MoveCard::getSteps() const { return steps; }

// Must implement functions
std::string MoveCard::getDescription() const {
  return "Move by " + std::to_string(steps) + " steps.";
}

std::string MoveCard::getType() const { return "MoveCard"; }

int MoveCard::getValue() const { return steps; }

void MoveCard::use(Player *p, GameManager *gm) {
  if (p == nullptr || gm == nullptr) {
    throw InternalGameException("MoveCard::use menerima konteks yang tidak valid.");
  }

  if (!p->canUseAbility()) {
    throw AbilityAlreadyUsedException();
  }

  // Pemain tidak dapat bergerak saat di penjara 
  if (p->isJailed()) {
    throw AbilityTimingException("MoveCard tidak dapat digunakan saat berada di Penjara.");
  }

  int boardSize = gm->getBoardSize();
  if (boardSize <= 0) {
    throw InternalGameException("MoveCard digunakan saat ukuran board tidak valid.");
  }

  if (p == &gm->getCurrentPlayer()) {
    gm->moveCurrentPlayer(steps);
  } else {
    int oldPos = p->getPosition();
    int newPos = (oldPos + steps) % boardSize;
    if (newPos < 0)
      newPos += boardSize;
    gm->movePlayerTo(*p, newPos, true);
  }

  Tile &tile = gm->getBoard().getTile(p->getPosition());
  std::cout << "Bidak mendarat di: " << tile.getName() << ".\n";

  markAsUsed();
  p->setUsedAbility();
  p->removeCard(this);

  gm->getLogger().log(gm->getCurrentTurn(), p->getUsername(), "KARTU", "MoveCard: Bergerak " + std::to_string(steps) + " langkah ke " + tile.getName());
  tile.onLanded(*p, *gm);
}
