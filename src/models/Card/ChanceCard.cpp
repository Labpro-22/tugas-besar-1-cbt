#include "ChanceCard.hpp"
#include "../../core/Board-Tiles/Board.hpp"
#include "../GameManager/GameManager.hpp"
#include "../GameManager/Player.hpp"

// ctor
ChanceCard::ChanceCard(int cardId, ChanceCardType type)
    : ActionCard(cardId), chanceType(type) {}

// dtor
ChanceCard::~ChanceCard() {}

// Must implement functions
std::string ChanceCard::getDescription() const {
  switch (chanceType) {
  case ChanceCardType::GO_TO_NEAREST_STATION:
    return "Go to nearest station.";
  case ChanceCardType::MOVE_BACK_3:
    return "Move back 3 tiles.";
  case ChanceCardType::GO_TO_JAIL:
    return "Go directly to jail.";
  default:
    return "Unknown chance card.";
  }
}

std::string ChanceCard::getType() const { return "ChanceCard"; }

void ChanceCard::execute(Player *p, GameManager *gm) {
  if (p == nullptr || gm == nullptr) {
    return;
  }

  int boardSize = gm->getBoardSize();

  switch (chanceType) {
  case ChanceCardType::GO_TO_NEAREST_STATION: {
    int destination = gm->getBoard().findNearestStation(p->getPosition());

    int oldPos = p->getPosition();
    int newPos = destination % boardSize;
    if (newPos < 0)
      newPos += boardSize;
    p->setPosition(newPos);
    if (newPos < oldPos) {
      p->addCash(gm->getGoSalary());
    }
    gm->addLogEntry(p->getUsername() + " menuju stasiun terdekat");
    break;
  }
  case ChanceCardType::MOVE_BACK_3: {
    int newPos = (p->getPosition() - 3) % boardSize;
    if (newPos < 0)
      newPos += boardSize;
    p->setPosition(newPos);
    gm->addLogEntry(p->getUsername() + " mundur 3 petak");
    break;
  }
  case ChanceCardType::GO_TO_JAIL: {
    p->setPosition(gm->getJailPosition());
    p->setStatus(JAILED);
    gm->addLogEntry(p->getUsername() + " dikirim ke penjara");
    break;
  }
  }
}
