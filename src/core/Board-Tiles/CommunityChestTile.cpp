#include "core/Board-Tiles/CommunityChestTile.hpp"
#include "models/Card/CommunityCard.hpp"
#include "models/Card/CommunityCardType.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "exception/NimonspoliExceptions.hpp"

#include <algorithm>
#include <random>
#include <vector>

std::vector<CommunityCardType> CommunityChestTile::makeCommunityDeck() {
  return {CommunityCardType::BIRTHDAY, CommunityCardType::DOCTOR_FEE,
          CommunityCardType::CAMPAIGN_FEE};
}

CommunityCardType CommunityChestTile::drawCommunityCardType() {
  static std::vector<CommunityCardType> deck;
  static std::mt19937 rng(std::random_device{}());

  if (deck.empty()) {
    deck = makeCommunityDeck();
    std::shuffle(deck.begin(), deck.end(), rng);
  }
  if (deck.empty()) {
    throw EmptyDeckException();
  }

  const CommunityCardType cardType = deck.back();
  deck.pop_back();
  return cardType;
}

CommunityChestTile::CommunityChestTile(const std::string &code,
                                       const std::string &name, int pos)
    : CardTile(code, name, pos, "community_chest") {}

void CommunityChestTile::drawCardandExecute(Player &player,
                                            GameManager &game) {
  try {
    CommunityCard card(0, drawCommunityCardType());

    logTileEvent(game, player, "KARTU",
                 "Mendarat di Dana Umum dan mengambil kartu: " +
                     card.getDescription());

    card.execute(&player, &game);
  } catch (const NimonspoliException &) {
    throw;
  } catch (const std::exception &e) {
    throw InternalGameException(std::string("CommunityChestTile::drawCardandExecute: ") + e.what());
  }
}

void CommunityChestTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
