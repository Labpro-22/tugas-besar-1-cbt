#include "../../../include/core/Board-Tiles/ChanceTile.hpp"
#include "../../../include/models/Card/ChanceCard.hpp"
#include "../../../include/models/Card/ChanceCardType.hpp"
#include "../../../include/models/GameManager/GameManager.hpp"
#include "../../../include/models/GameManager/Player.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

namespace {
std::vector<ChanceCardType> makeChanceDeck() {
  return {ChanceCardType::GO_TO_NEAREST_STATION, ChanceCardType::MOVE_BACK_3,
          ChanceCardType::GO_TO_JAIL};
}

ChanceCardType drawChanceCardType() {
  static std::vector<ChanceCardType> deck;
  static std::mt19937 rng(std::random_device{}());

  if (deck.empty()) {
    deck = makeChanceDeck();
    std::shuffle(deck.begin(), deck.end(), rng);
  }

  ChanceCardType cardType = deck.back();
  deck.pop_back();
  return cardType;
}
} // namespace

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
