#include "core/Board-Tiles/GoTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"

#include <type_traits>
#include <utility>

namespace {
    template <typename PlayerType, typename = void>
    struct HasReceiveMoney : std::false_type {};

    template <typename PlayerType>
    struct HasReceiveMoney<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().receiveMoney(0))>> : std::true_type {};

    template <typename PlayerType, typename = void>
    struct HasAddMoney : std::false_type {};

    template <typename PlayerType>
    struct HasAddMoney<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().addMoney(0))>> : std::true_type {};

    template <typename PlayerType, typename = void>
    struct HasDepositMoney : std::false_type {};

    template <typename PlayerType>
    struct HasDepositMoney<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().depositMoney(0))>> : std::true_type {};

    template <typename PlayerType, typename = void>
    struct HasEarnMoney : std::false_type {};

    template <typename PlayerType>
    struct HasEarnMoney<PlayerType, std::void_t<decltype(std::declval<PlayerType &>().earnMoney(0))>> : std::true_type {};

    template <typename GameType, typename PlayerType, typename = void>
    struct HasExecuteSalary : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasExecuteSalary<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().executeSalary(std::declval<PlayerType &>(), 0))>> : std::true_type {};

    template <typename GameType, typename PlayerType, typename = void>
    struct HasAwardSalary : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasAwardSalary<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().awardSalary(std::declval<PlayerType &>(), 0))>> : std::true_type {};

    template <typename GameType, typename PlayerType, typename = void>
    struct HasCollectSalary : std::false_type {};

    template <typename GameType, typename PlayerType>
    struct HasCollectSalary<GameType, PlayerType, std::void_t<decltype(std::declval<GameType &>().collectSalary(std::declval<PlayerType &>(), 0))>> : std::true_type {};

    template <typename GameType, typename PlayerType>
    void awardGoSalary(GameType &game, PlayerType &player, int salary) {
        if constexpr (HasExecuteSalary<GameType, PlayerType>::value) {
            game.executeSalary(player, salary);
        }
        else if constexpr (HasAwardSalary<GameType, PlayerType>::value) {
            game.awardSalary(player, salary);
        }
        else if constexpr (HasCollectSalary<GameType, PlayerType>::value) {
            game.collectSalary(player, salary);
        }
        else if constexpr (HasReceiveMoney<PlayerType>::value) {
            player.receiveMoney(salary);
        }
        else if constexpr (HasAddMoney<PlayerType>::value) {
            player.addMoney(salary);
        }
        else if constexpr (HasDepositMoney<PlayerType>::value) {
            player.depositMoney(salary);
        }
        else if constexpr (HasEarnMoney<PlayerType>::value) {
            player.earnMoney(salary);
        }
    }
}

GoTile::GoTile(const std::string &code, const std::string &name, int pos, int salary)
    : ActionTile(code, name, pos, "go"), salary(salary) {}

void GoTile::onLanded(Player &player, GameManager &game) {
    awardGoSalary(game, player, getSalary());
}

void GoTile::onPassed(Player &player, GameManager &game) {
    awardGoSalary(game, player, getSalary());
}

int GoTile::getSalary() const { return salary; }
