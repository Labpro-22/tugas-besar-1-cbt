#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "Player.hpp"
using namespace std;


class PropertyState {
    private:
    int fmult;
    int fdur;
    int buildingLevel;
    public:
        PropertyState(int fmult, int fdur, int buildingLevel);
        int getFmult() const;
        int getFdur() const;
        int getBuildLevel() const;
};