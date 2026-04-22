#include "core/Board-Tiles/ChanceTile.hpp"
#include "models/Card/ChanceCard.hpp"
#include "models/Card/ChanceCardType.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "exception/NimonspoliExceptions.hpp"

#include <algorithm>
#include <random>
#include <vector>

std::vector<ChanceCardType> ChanceTile::makeChanceDeck() {
  return {ChanceCardType::GO_TO_NEAREST_STATION, ChanceCardType::MOVE_BACK_3,
          ChanceCardType::GO_TO_JAIL};
}

ChanceCardType ChanceTile::drawChanceCardType() {
  static std::vector<ChanceCardType> deck;
  static std::mt19937 rng(std::random_device{}());

  if (deck.empty()) {
    deck = makeChanceDeck();
    std::shuffle(deck.begin(), deck.end(), rng);
  }
  if (deck.empty()) {
    throw EmptyDeckException();
  }

  const ChanceCardType cardType = deck.back();
  deck.pop_back();
  return cardType;
}

ChanceTile::ChanceTile(const std::string &code, const std::string &name,
                       int pos)
    : CardTile(code, name, pos, "chance") {}

void ChanceTile::drawCardandExecute(Player &player, GameManager &game) {
  try {
    ChanceCard card(0, drawChanceCardType());

    logTileEvent(game, player, "KARTU",
                 "Mendarat di Kesempatan dan mengambil kartu: " +
                     card.getDescription());

    card.execute(&player, &game);
  } catch (const NimonspoliException &) {
    throw;
  } catch (const std::exception &e) {
    throw InternalGameException(std::string("ChanceTile::drawCardandExecute: ") + e.what());
  }
}

void ChanceTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
