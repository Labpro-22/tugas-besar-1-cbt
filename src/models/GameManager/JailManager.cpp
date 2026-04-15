#include "JailManager.hpp"

JailManager::JailManager(int maxJailTurns, int jailFne) : maxJailTurns(maxJailTurns), jailFine(jailFine) {};

void JailManager::sendToJail(Player& player){
    prisoners.push_back(player);
    jailTurns[player] = maxJailTurns;
}
void JailManager::releasePlayer(Player& player){
    if (isInJail(player)) {
        jailTurns.erase(player);
        for (auto it = prisoners.begin(); it != prisoners.end(); ++it) {
            if (it->getUsername() == player.getUsername()) {
                prisoners.erase(it);
                break;
            }
        }
        player.setStatus(0);
    }
}
void JailManager::incrementJailTurn(){
    for(auto it = prisoners.begin(); it != prisoners.end(); ++it){
        jailTurns[*it]--;
    }
}
bool JailManager::canReleaseByFine(Player& player){
    return player.getCash() >= jailFine;
}
bool JailManager::canReleaseByDouble(Player& player) {
    if (isInJail(player)) {
        return jailTurns[player] < maxJailTurns;
    }
    return false;
}
bool JailManager::mustPayFine(Player& player) {
    if (isInJail(player)) {
        return jailTurns[player] >= maxJailTurns;
    }
    return false;
}
int JailManager::getJailTurns(Player& player){
    return jailTurns[player];
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
    return jailTurns.find(player) != jailTurns.end();
}