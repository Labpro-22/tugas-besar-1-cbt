#include "JailManager.hpp"
#include "Player.hpp"

JailManager::JailManager(int maxJailTurns, int jailFne) : maxJailTurns(maxJailTurns), jailFine(jailFne) {};

void JailManager::sendToJail(Player& player){
    if (!isInJail(player)) {
        prisoners.push_back(player);
    }
    jailTurns[player.getUsername()] = maxJailTurns;
    player.setStatus(JAILED);
}
void JailManager::releasePlayer(Player& player){
    if (isInJail(player)) {
        jailTurns.erase(player.getUsername());
        for (auto it = prisoners.begin(); it != prisoners.end(); ++it) {
            if (it->getUsername() == player.getUsername()) {
                prisoners.erase(it);
                break;
            }
        }
        player.setStatus(ACTIVE);
    }
}
void JailManager::incrementJailTurn(){
    for(auto it = prisoners.begin(); it != prisoners.end(); ++it){
        auto turnIt = jailTurns.find(it->getUsername());
        if (turnIt != jailTurns.end() && turnIt->second > 0) {
            turnIt->second--;
        }
    }
}
bool JailManager::canReleaseByFine(Player& player){
    return player.getCash() >= jailFine;
}
bool JailManager::canReleaseByDouble(Player& player) {
    if (isInJail(player)) {
        return jailTurns[player.getUsername()] < maxJailTurns;
    }
    return false;
}
bool JailManager::mustPayFine(Player& player) {
    if (isInJail(player)) {
        return jailTurns[player.getUsername()] >= maxJailTurns;
    }
    return false;
}
int JailManager::getJailTurns(Player& player){
    auto it = jailTurns.find(player.getUsername());
    if (it == jailTurns.end()) {
        return 0;
    }
    return it->second;
}
bool JailManager::payFine(Player& player){
    if(canReleaseByFine(player)){
        player.reduceCash(jailFine);
        releasePlayer(player);
        return true;
    }
    return false;
}
bool JailManager::isInJail(Player& player){
    return jailTurns.find(player.getUsername()) != jailTurns.end();
}