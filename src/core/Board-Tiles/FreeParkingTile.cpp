#include "core/Board-Tiles/FreeParkingTile.hpp"
#include "core/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <type_traits>
#include <utility>

namespace {
  template <typename GameType, typename PlayerType, typename = void>
  struct HasCollectFreeParking : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasCollectFreeParking<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().collectFreeParking(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename GameType, typename PlayerType, typename = void>
  struct HasExecuteFreeParking : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasExecuteFreeParking<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().executeFreeParking(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename GameType, typename PlayerType, typename = void>
  struct HasResolveFreeParking : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasResolveFreeParking<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().resolveFreeParking(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename GameType, typename PlayerType, typename = void>
  struct HasAddFreeParkingPool : std::false_type {};

  template <typename GameType, typename PlayerType>
  struct HasAddFreeParkingPool<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().addFreeParkingPool(std::declval<PlayerType &>()))>>
    : std::true_type {};

  template <typename GameType, typename PlayerType>
  void handleFreeParking(GameType &game, PlayerType &player) {
    if constexpr (HasCollectFreeParking<GameType, PlayerType>::value) {
      game.collectFreeParking(player);
    }
    else if constexpr (HasExecuteFreeParking<GameType, PlayerType>::value) {
      game.executeFreeParking(player);
    }
    else if constexpr (HasResolveFreeParking<GameType, PlayerType>::value) {
      game.resolveFreeParking(player);
    }
    else if constexpr (HasAddFreeParkingPool<GameType, PlayerType>::value) {
      game.addFreeParkingPool(player);
    }
  }
}

FreeParkingTile::FreeParkingTile(const std::string &code, const std::string &name, int pos)
  : ActionTile(code, name, pos, "free_parking") {}

void FreeParkingTile::onLanded(Player &player, GameManager &game) {
  handleFreeParking(game, player);
}
