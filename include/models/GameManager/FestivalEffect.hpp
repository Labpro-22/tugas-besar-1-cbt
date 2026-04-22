#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class FestivalEffect {
private:
    map<string, pair<int, int>> effects;

public:
    int activateEffect(string propertyCode, int currentFMULT);
    void tickDuration(string propertyCode);
    int getMultiplier(string propertyCode);
    int getDuration(string propertyCode);
    bool isMultiplierActive(string propertyCode);
    bool isMaxMultiplier(string propertyCode);
    void clearExpired();
};