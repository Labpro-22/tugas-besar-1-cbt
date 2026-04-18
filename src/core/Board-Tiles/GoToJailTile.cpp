#include "core/Board-Tiles/GoToJailTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <type_traits>
#include <utility>

namespace {
    template <typename GameType, typename PlayerType, typename = void>
    struct HasGoToJail : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasGoToJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().goToJail(std::declval<PlayerType &>()))>>
        : std::true_type {};

    template <typename GameType, typename PlayerType, typename = void>
    struct HasSendPlayerToJail : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasSendPlayerToJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().sendPlayerToJail(std::declval<PlayerType &>()))>>
        : std::true_type {};

    template <typename GameType, typename PlayerType, typename = void>
    struct HasMovePlayerToJail : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasMovePlayerToJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().movePlayerToJail(std::declval<PlayerType &>()))>>
        : std::true_type {};

    template <typename GameType, typename PlayerType, typename = void>
    struct HasExecuteGoToJail : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasExecuteGoToJail<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().executeGoToJail(std::declval<PlayerType &>()))>>
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
    struct HasGoToJailState : std::false_type {};

    template <typename PlayerType>
    struct HasGoToJailState<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().goToJail())>> : std::true_type {};

    template <typename GameType, typename PlayerType>
    void movePlayerToJail(GameType &game, PlayerType &player) {
        if constexpr (HasGoToJail<GameType, PlayerType>::value) {
            game.goToJail(player);
        }
        else if constexpr (HasSendPlayerToJail<GameType, PlayerType>::value) {
            game.sendPlayerToJail(player);
        }
        else if constexpr (HasMovePlayerToJail<GameType, PlayerType>::value) {
            game.movePlayerToJail(player);
        }
        else if constexpr (HasExecuteGoToJail<GameType, PlayerType>::value) {
            game.executeGoToJail(player);
        }
        else if constexpr (HasSetInJail<PlayerType>::value) {
            player.setInJail(true);
        }
        else if constexpr (HasSetJailStatus<PlayerType>::value) {
            player.setJailStatus(true);
        }
        else if constexpr (HasGoToJailState<PlayerType>::value) {
            player.goToJail();
        }
    }
}

GoToJailTile::GoToJailTile(const std::string &code, const std::string &name, int pos)
    : ActionTile(code, name, pos, "go_to_jail") {}

void GoToJailTile::onLanded(Player &player, GameManager &game) {
    movePlayerToJail(game, player);
}
