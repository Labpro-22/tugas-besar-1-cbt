#include "models/GameManager/FestivalEffect.hpp"
int FestivalEffect::activateEffect(string propertyCode, int currentFMULT) {
    int newFMULT;

    if (effects.find(propertyCode) == effects.end()) {
        newFMULT = (currentFMULT < 2) ? 2 : currentFMULT * 2;
    } 
    else {
        newFMULT = currentFMULT * 2;
    }

    if (newFMULT > 8) {
        newFMULT = 8;
    }

    effects[propertyCode] = make_pair(newFMULT, 3);
    
    return newFMULT;
}

void FestivalEffect::tickDuration(string propertyCode) {
    auto it = effects.find(propertyCode);
    
    if (it != effects.end()) {
        if (it->second.second > 0) {
            it->second.second--;
        }
    }
}

int FestivalEffect::getMultiplier(string propertyCode) {
    auto it = effects.find(propertyCode);
    if (it != effects.end() && it->second.second > 0) {
        return it->second.first;
    }
    return 1;
}

int FestivalEffect::getDuration(string propertyCode) {
    auto it = effects.find(propertyCode);
    if (it != effects.end()) {
        return it->second.second;
    }
    return 0;
}

bool FestivalEffect::isMultiplierActive(string propertyCode) {
    return getDuration(propertyCode) > 0;
}

bool FestivalEffect::isMaxMultiplier(string propertyCode) {
    return getMultiplier(propertyCode) >= 8;
}

void FestivalEffect::clearExpired() {
    for (auto it = effects.begin(); it != effects.end(); ) {
        if (it->second.second <= 0) {
            it = effects.erase(it);
        } else {
            ++it;
        }
    }
}