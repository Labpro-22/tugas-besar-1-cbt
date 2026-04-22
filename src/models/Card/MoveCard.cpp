#include "models/Card/MoveCard.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/Tile.hpp"

#include <iostream>

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
    return;
  }

  if (!p->canUseAbility()) {
    gm->addLogEntry(p->getUsername() + " gagal menggunakan MoveCard (ability sudah dipakai)");
    return;
  }

  int boardSize = gm->getBoardSize();
  if (boardSize <= 0) {
    gm->addLogEntry("MoveCard gagal: ukuran board tidak valid");
    return;
  }

  std::cout << "MoveCard diaktifkan! Bergerak maju " << steps << " petak.\n";

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

  gm->addLogEntry("MoveCard digunakan sebanyak " + std::to_string(steps) + " langkah");
  tile.onLanded(*p, *gm);
}
