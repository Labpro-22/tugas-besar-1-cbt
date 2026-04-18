#include "core/JailTile.hpp"

#include <type_traits>
#include <utility>

namespace {
  template <typename GameType, typename PlayerType, typename = void>
  struct HasVisitJail : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasVisitJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().visitJail(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename GameType, typename PlayerType, typename = void>
  struct HasEnterJail : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasEnterJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().enterJail(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename GameType, typename PlayerType, typename = void>
  struct HasHandleJail : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasHandleJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().handleJail(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename PlayerType, typename = void>
  struct HasSetInJail : std::false_type {};

  template <typename PlayerType>
  struct HasSetInJail<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().setInJail(true))>> : std::true_type {};

  template <typename PlayerType, typename = void>
  struct HasSetJailStatus : std::false_type {};

  template <typename PlayerType>
  struct HasSetJailStatus<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().setJailStatus(true))>> : std::true_type {};

  template <typename PlayerType, typename = void>
  struct HasSetVisitedJail : std::false_type {};

  template <typename PlayerType>
  struct HasSetVisitedJail<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().setVisitedJail(true))>> : std::true_type {};

  template <typename GameType, typename PlayerType>
  void handleJailVisit(GameType &game, PlayerType &player) {
    if constexpr (HasVisitJail<GameType, PlayerType>::value) {
      game.visitJail(player);
    }
    else if constexpr (HasEnterJail<GameType, PlayerType>::value) {
      game.enterJail(player);
    }
    else if constexpr (HasHandleJail<GameType, PlayerType>::value) {
      game.handleJail(player);
    }
    else if constexpr (HasSetInJail<PlayerType>::value) {
      player.setInJail(true);
    }
    else if constexpr (HasSetJailStatus<PlayerType>::value) {
      player.setJailStatus(true);
    }
    else if constexpr (HasSetVisitedJail<PlayerType>::value) {
      player.setVisitedJail(true);
    }
  }
}

JailTile::JailTile(const std::string &code, const std::string &name, int pos)
  : ActionTile(code, name, pos, "jail") {}

void JailTile::onLanded(Player &player, GameManager &game) {
  handleJailVisit(game, player);
}
