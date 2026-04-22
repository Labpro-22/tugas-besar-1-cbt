#pragma once
#include <iostream>
#include <random>
using namespace std;

class Dice {
private:
    int dice1;
    int dice2;
    int consecutiveDouble;

public:
    Dice();
    int roll();
    void setValues(int d1, int d2);
    int getTotal();
    int getDie1() const;
    int getDie2() const;
    bool checkDouble();
    int getConsecutiveDoubles() const;
    void resetConsecutiveDoubles();
    bool hasThreeConsecutiveDoubles() const;
};