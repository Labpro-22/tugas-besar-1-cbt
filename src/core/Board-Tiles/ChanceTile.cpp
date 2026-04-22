#include "core/Board-Tiles/ChanceTile.hpp"
#include "models/Card/ChanceCard.hpp"
#include "models/Card/ChanceCardType.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

static std::vector<ChanceCardType> makeChanceDeck() {
  return {ChanceCardType::GO_TO_NEAREST_STATION, ChanceCardType::MOVE_BACK_3,
          ChanceCardType::GO_TO_JAIL};
}

static ChanceCardType drawChanceCardType() {
  static std::vector<ChanceCardType> deck;
  static std::mt19937 rng(std::random_device{}());

  if (deck.empty()) {
    deck = makeChanceDeck();
    std::shuffle(deck.begin(), deck.end(), rng);
  }

  const ChanceCardType cardType = deck.back();
  deck.pop_back();
  return cardType;
}

ChanceTile::ChanceTile(const std::string &code, const std::string &name,
                       int pos)
    : CardTile(code, name, pos, "chance") {}

void ChanceTile::drawCardandExecute(Player &player, GameManager &game) {
  ChanceCard card(0, drawChanceCardType());

  std::cout << "Kamu mendarat di Petak Kesempatan!\n";
  std::cout << "Mengambil kartu...\n";
  std::cout << "Kartu: \"" << card.getDescription() << "\"\n";

  card.execute(&player, &game);
}

void ChanceTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
