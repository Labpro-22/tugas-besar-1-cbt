#include "core/Board-Tiles/CommunityChestTile.hpp"
#include "models/Card/CommunityCard.hpp"
#include "models/Card/CommunityCardType.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

namespace {
std::vector<CommunityCardType> makeCommunityDeck() {
  return {CommunityCardType::BIRTHDAY, CommunityCardType::DOCTOR_FEE,
          CommunityCardType::CAMPAIGN_FEE};
}

CommunityCardType drawCommunityCardType() {
  static std::vector<CommunityCardType> deck;
  static std::mt19937 rng(std::random_device{}());

  if (deck.empty()) {
    deck = makeCommunityDeck();
    std::shuffle(deck.begin(), deck.end(), rng);
  }

  CommunityCardType cardType = deck.back();
  deck.pop_back();
  return cardType;
}
} // namespace

CommunityChestTile::CommunityChestTile(const std::string &code,
                                       const std::string &name, int pos)
    : CardTile(code, name, pos, "community_chest") {}

void CommunityChestTile::drawCardandExecute(Player &player,
                                            GameManager &game) {
  CommunityCard card(0, drawCommunityCardType());

  std::cout << "Kamu mendarat di Petak Dana Umum!\n";
  std::cout << "Mengambil kartu...\n";
  std::cout << "Kartu: \"" << card.getDescription() << "\"\n";

  card.execute(&player, &game);
}

void CommunityChestTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
