#include "JailManager.hpp"
#include "Player.hpp"

JailManager::JailManager() : maxJailTurns(3), jailFine(50) {}

JailManager::JailManager(int maxJailTurns, int jailFine)
    : maxJailTurns(maxJailTurns), jailFine(jailFine) {}

void JailManager::sendToJail(Player &player) {
    prisoners.push_back(&player);
    jailTurns[player.getUsername()] = 0;
    player.setStatus(JAILED);
}
void JailManager::releasePlayer(Player &player) {
    if (isInJail(player)) {
        jailTurns.erase(player.getUsername());
        for (auto it = prisoners.begin(); it != prisoners.end(); ++it) {
            if ((*it)->getUsername() == player.getUsername()) {
                prisoners.erase(it);
                break;
            }
        }
        player.setStatus(ACTIVE);
    }
}
void JailManager::incrementJailTurn() {
    for (auto it = prisoners.begin(); it != prisoners.end(); ++it) {
        jailTurns[(*it)->getUsername()]++;
    }
}
bool JailManager::canReleaseByFine(Player &player) {
    return player.getCash() >= jailFine;
}
bool JailManager::canReleaseByDouble(Player &player) {
    if (isInJail(player)) {
        return jailTurns[player.getUsername()] < maxJailTurns;
    }
    return false;
}
bool JailManager::mustPayFine(Player &player) {
    if (isInJail(player)) {
        return jailTurns[player.getUsername()] >= maxJailTurns;
    }
    return false;
}
int JailManager::getJailTurns(Player &player) {
    return jailTurns[player.getUsername()];
}
bool JailManager::payFine(Player &player) {
    if (canReleaseByFine(player)) {
        player.reduceCash(jailFine);
        releasePlayer(player);
        return true;
    }
    return false;
}
bool JailManager::isInJail(Player &player) {
    return jailTurns.find(player.getUsername()) != jailTurns.end();
}