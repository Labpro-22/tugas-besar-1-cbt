#include "models/Card/ChanceCard.hpp"
#include "core/Board-Tiles/Board.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

// ctor
ChanceCard::ChanceCard(int cardId, ChanceCardType type)
    : ActionCard(cardId), chanceType(type) {}

// dtor
ChanceCard::~ChanceCard() {}

// Must implement functions
std::string ChanceCard::getDescription() const {
  switch (chanceType) {
  case ChanceCardType::GO_TO_NEAREST_STATION:
    return "Pergi ke stasiun terdekat.";
  case ChanceCardType::MOVE_BACK_3:
    return "Mundur 3 petak.";
  case ChanceCardType::GO_TO_JAIL:
    return "Masuk Penjara.";
  default:
    return "Kartu kesempatan tidak dikenal.";
  }
}

std::string ChanceCard::getType() const { return "ChanceCard"; }

void ChanceCard::execute(Player *p, GameManager *gm) {
  if (p == nullptr || gm == nullptr) {
    return;
  }

  int boardSize = gm->getBoardSize();
  if (boardSize <= 0) {
    gm->addLogEntry("ChanceCard gagal: ukuran board tidak valid");
    return;
  }

  switch (chanceType) {
  case ChanceCardType::GO_TO_NEAREST_STATION: {
    int newPos = gm->getBoard().findNearestStation(p->getPosition());
    newPos = ((newPos % boardSize) + boardSize) % boardSize;
    gm->movePlayerTo(*p, newPos, true);
    Tile &tile = gm->getBoard().getTile(newPos);
    std::cout << "Bidak dipindahkan ke " << tile.getName() << ".\n";
    gm->addLogEntry(p->getUsername() + " menuju stasiun terdekat: " + tile.getName());
    tile.onLanded(*p, *gm);
    break;
  }
  case ChanceCardType::MOVE_BACK_3: {
    int newPos = ((p->getPosition() - 3) % boardSize + boardSize) % boardSize;
    p->setPosition(newPos);
    Tile &tile = gm->getBoard().getTile(newPos);
    std::cout << "Bidak dipindahkan ke " << tile.getName() << ".\n";
    gm->addLogEntry(p->getUsername() + " mundur 3 petak ke " + tile.getName());
    tile.onLanded(*p, *gm);
    break;
  }
  case ChanceCardType::GO_TO_JAIL: {
    gm->goToJail(*p);
    break;
  }
  }
}
