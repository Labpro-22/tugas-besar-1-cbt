#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
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