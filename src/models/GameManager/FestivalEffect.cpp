#include "FestivalEffect.hpp"

int FestivalEffect::activateEffect(string propertyCode, int currentFMULT) {
    for (auto& pair : effects) {
        if (pair.first.getCityCode() == propertyCode) {
            pair.second = 3;
            return currentFMULT * 2; 
        }
    }
    return currentFMULT;
}

void FestivalEffect::tickDuration(string propertyCode) {
    for (auto& pair : effects) {
        if (pair.first.getCityCode() == propertyCode) {
            if (pair.second > 0) {
                pair.second--;
            }
            break;
        }
    }
}

int FestivalEffect::getMultiplier(string propertyCode) {
    if (isMultiplierActive(propertyCode)) {
        return 2; 
    return 1;
    }
}

int FestivalEffect::getDuration(string propertyCode) {
    for (auto const& [prop, duration] : effects) {
        if (prop.getCityCode() == propertyCode) {
            return duration;
        }
    }
    return 0;
}

bool FestivalEffect::isMultiplierActive(string propertyCode) {
    return getDuration(propertyCode) > 0;
}

bool FestivalEffect::isMaxMultiplier(string propertyCode) {
    return getMultiplier(propertyCode) >= 4;
}

void FestivalEffect::clearExpired() {
    for (auto it = effects.begin(); it != effects.end(); ) {
        if (it->second <= 0) {
            it = effects.erase(it);
        } else {
            ++it;
        }
    }
}