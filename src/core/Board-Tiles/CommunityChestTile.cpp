#include "core/Board-Tiles/CommunityChestTile.hpp"
#include "core/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <type_traits>
#include <utility>

namespace {
  template <typename T, typename = void>
  struct HasDrawCommunityChestCard : std::false_type {};

  template <typename T>
  struct HasDrawCommunityChestCard<T, std::void_t<decltype(std::declval<T &>().drawCommunityChestCard())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasGetCommunityChestDeck : std::false_type {};

  template <typename T>
  struct HasGetCommunityChestDeck<T, std::void_t<decltype(std::declval<T &>().getCommunityChestDeck())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasDeckDraw : std::false_type {};

  template <typename T>
  struct HasDeckDraw<T, std::void_t<decltype(std::declval<T &>().draw())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasDeckIsEmpty : std::false_type {};

  template <typename T>
  struct HasDeckIsEmpty<T, std::void_t<decltype(std::declval<const T &>().isEmpty())>> : std::true_type {};

  template <typename GameType, typename CardType, typename = void>
  struct HasExecuteCommunityChestCard : std::false_type {};

  template <typename GameType, typename CardType>
  struct HasExecuteCommunityChestCard<GameType, CardType, std::void_t<decltype(std::declval<GameType &>().executeCommunityChestCard(std::declval<Player &>(), std::declval<CardType &>()))>>
    : std::true_type {};

  template <typename GameType, typename CardType, typename = void>
  struct HasResolveCommunityChestCard : std::false_type {};

  template <typename GameType, typename CardType>
  struct HasResolveCommunityChestCard<GameType, CardType, std::void_t<decltype(std::declval<GameType &>().resolveCommunityChestCard(std::declval<Player &>(), std::declval<CardType &>()))>>
    : std::true_type {};

  template <typename CardType, typename = void>
  struct HasCardExecute : std::false_type {};

  template <typename CardType>
  struct HasCardExecute<CardType, std::void_t<decltype(std::declval<CardType &>().execute(std::declval<Player &>(), std::declval<GameManager &>()))>>
    : std::true_type {};

  template <typename CardType, typename = void>
  struct HasCardApply : std::false_type {};

  template <typename CardType>
  struct HasCardApply<CardType, std::void_t<decltype(std::declval<CardType &>().apply(std::declval<Player &>(), std::declval<GameManager &>()))>>
    : std::true_type {};

  template <typename GameType, typename CardType, typename = void>
  struct HasDiscardCommunityChestCard : std::false_type {};

  template <typename GameType, typename CardType>
  struct HasDiscardCommunityChestCard<GameType, CardType, std::void_t<decltype(std::declval<GameType &>().discardCommunityChestCard(std::declval<const CardType &>()))>>
    : std::true_type {};

  template <typename DeckType, typename CardType, typename = void>
  struct HasDeckDiscardCard : std::false_type {};

  template <typename DeckType, typename CardType>
  struct HasDeckDiscardCard<DeckType, CardType, std::void_t<decltype(std::declval<DeckType &>().discardCard(std::declval<const CardType &>()))>>
    : std::true_type {};

  template <typename CardType>
  void executeCommunityChestCardEffect(Player &player, GameManager &game, CardType &card) {
    if constexpr (HasCardExecute<CardType>::value) {
      card.execute(player, game);
    } 
    else if constexpr (HasCardApply<CardType>::value) {
      card.apply(player, game);
    } 
    else if constexpr (HasExecuteCommunityChestCard<GameManager, CardType>::value) {
      game.executeCommunityChestCard(player, card);
    }
    else if constexpr (HasResolveCommunityChestCard<GameManager, CardType>::value) {
      game.resolveCommunityChestCard(player, card);
    }
  }

  template <typename CardType>
  void discardCommunityChestCardToGame(GameManager &game, const CardType &card) {
    if constexpr (HasDiscardCommunityChestCard<GameManager, CardType>::value) {
      game.discardCommunityChestCard(card);
    }
  }

  template <typename DeckType, typename CardType>
  void discardCommunityChestCardToDeck(DeckType &deck, const CardType &card) {
    if constexpr (HasDeckDiscardCard<DeckType, CardType>::value) {
      deck.discardCard(card);
    }
  }

  template <typename CardHandle>
  auto *toCardPtr(CardHandle &card) {
    if constexpr (std::is_pointer_v<std::decay_t<CardHandle>>) {
      return card;
    } else {
      return &card;
    }
  }

  template <typename DeckType>
  bool drawFromDeckAndExecute(Player &player, GameManager &game, DeckType &deck) {
    if constexpr (!HasDeckDraw<DeckType>::value) {
      return false;
    }
    else {
      if constexpr (HasDeckIsEmpty<DeckType>::value) {
        if (deck.isEmpty()) {
          return false;
        }
      }

      auto card = deck.draw();
      auto *cardPtr = toCardPtr(card);
      if (cardPtr == nullptr) {
        return false;
      }

      executeCommunityChestCardEffect(player, game, *cardPtr);
      discardCommunityChestCardToDeck(deck, *cardPtr);
      discardCommunityChestCardToGame(game, *cardPtr);
      return true;
    }
  }

  template <typename DeckType>
  bool drawFromDeckHandle(Player &player, GameManager &game, DeckType *deckPtr) {
    if (deckPtr == nullptr) {
      return false;
    }
    return drawFromDeckAndExecute(player, game, *deckPtr);
  }

  template <typename DeckType>
  bool drawFromDeckHandle(Player &player, GameManager &game, DeckType &deckRef) {
    return drawFromDeckAndExecute(player, game, deckRef);
  }

  bool drawFromGameManagerCommunityChest(Player &player, GameManager &game) {
    if constexpr (HasDrawCommunityChestCard<GameManager>::value) {
      auto card = game.drawCommunityChestCard();
      auto *cardPtr = toCardPtr(card);
      if (cardPtr == nullptr) {
        return false;
      }

      executeCommunityChestCardEffect(player, game, *cardPtr);
      discardCommunityChestCardToGame(game, *cardPtr);
      return true;
    }

    if constexpr (HasGetCommunityChestDeck<GameManager>::value) {
      auto deckHandle = game.getCommunityChestDeck();
      return drawFromDeckHandle(player, game, deckHandle);
    }

    return false;
  }
}

CommunityChestTile::CommunityChestTile(const std::string &code, const std::string &name, int pos)
  : CardTile(code, name, pos, "community_chest") {}

void CommunityChestTile::drawCardandExecute(Player &player, GameManager &game) {
  drawFromGameManagerCommunityChest(player, game);
}

void CommunityChestTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
