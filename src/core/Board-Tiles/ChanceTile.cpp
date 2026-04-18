#include "core/Board-Tiles/ChanceTile.hpp"
#include "core/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <type_traits>
#include <utility>

namespace {
  template <typename T, typename = void>
  struct HasDrawChanceCard : std::false_type {};

  template <typename T>
  struct HasDrawChanceCard<T, std::void_t<decltype(std::declval<T &>().drawChanceCard())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasGetChanceDeck : std::false_type {};

  template <typename T>
  struct HasGetChanceDeck<T, std::void_t<decltype(std::declval<T &>().getChanceDeck())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasDeckDraw : std::false_type {};

  template <typename T>
  struct HasDeckDraw<T, std::void_t<decltype(std::declval<T &>().draw())>> : std::true_type {};

  template <typename T, typename = void>
  struct HasDeckIsEmpty : std::false_type {};

  template <typename T>
  struct HasDeckIsEmpty<T, std::void_t<decltype(std::declval<const T &>().isEmpty())>> : std::true_type {};

  template <typename GameType, typename CardType, typename = void>
  struct HasExecuteChanceCard : std::false_type {};

  template <typename GameType, typename CardType>
  struct HasExecuteChanceCard<GameType, CardType, std::void_t<decltype(std::declval<GameType &>().executeChanceCard(std::declval<Player &>(), std::declval<CardType &>()))>>
    : std::true_type {};

  template <typename GameType, typename CardType, typename = void>
  struct HasResolveChanceCard : std::false_type {};

  template <typename GameType, typename CardType>
  struct HasResolveChanceCard<GameType, CardType, std::void_t<decltype(std::declval<GameType &>().resolveChanceCard(std::declval<Player &>(), std::declval<CardType &>()))>>
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
  struct HasDiscardChanceCard : std::false_type {};

  template <typename GameType, typename CardType>
  struct HasDiscardChanceCard<GameType, CardType, std::void_t<decltype(std::declval<GameType &>().discardChanceCard(std::declval<const CardType &>()))>>
    : std::true_type {};

  template <typename DeckType, typename CardType, typename = void>
  struct HasDeckDiscardCard : std::false_type {};

  template <typename DeckType, typename CardType>
  struct HasDeckDiscardCard<DeckType, CardType, std::void_t<decltype(std::declval<DeckType &>().discardCard(std::declval<const CardType &>()))>>
    : std::true_type {};

  template <typename CardType>
  void executeChanceCardEffect(Player &player, GameManager &game, CardType &card) {
    if constexpr (HasCardExecute<CardType>::value) {
      card.execute(player, game);
    } 
    else if constexpr (HasCardApply<CardType>::value) {
      card.apply(player, game);
    } 
    else if constexpr (HasExecuteChanceCard<GameManager, CardType>::value) {
      game.executeChanceCard(player, card);
    } 
    else if constexpr (HasResolveChanceCard<GameManager, CardType>::value) {
      game.resolveChanceCard(player, card);
    }
  }

  template <typename CardType>
  void discardChanceCardToGame(GameManager &game, const CardType &card) {
    if constexpr (HasDiscardChanceCard<GameManager, CardType>::value) {
      game.discardChanceCard(card);
    }
  }

  template <typename DeckType, typename CardType>
  void discardChanceCardToDeck(DeckType &deck, const CardType &card) {
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

      executeChanceCardEffect(player, game, *cardPtr);
      discardChanceCardToDeck(deck, *cardPtr);
      discardChanceCardToGame(game, *cardPtr);
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

  bool drawFromGameManagerChance(Player &player, GameManager &game) {
    if constexpr (HasDrawChanceCard<GameManager>::value) {
      auto card = game.drawChanceCard();
      auto *cardPtr = toCardPtr(card);
      if (cardPtr == nullptr) {
        return false;
      }

      executeChanceCardEffect(player, game, *cardPtr);
      discardChanceCardToGame(game, *cardPtr);
      return true;
    }

    if constexpr (HasGetChanceDeck<GameManager>::value) {
      auto deckHandle = game.getChanceDeck();
      return drawFromDeckHandle(player, game, deckHandle);
    }

    return false;
  }
}

ChanceTile::ChanceTile(const std::string &code, const std::string &name,int pos)
  : CardTile(code, name, pos, "chance") {}

void ChanceTile::drawCardandExecute(Player &player, GameManager &game) {
  drawFromGameManagerChance(player, game);
}

void ChanceTile::onLanded(Player &player, GameManager &game) {
  drawCardandExecute(player, game);
}
