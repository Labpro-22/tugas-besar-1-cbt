#pragma once
#include <iostream>
#include <random>
using namespace std;

class Dice {
    private :
        int dice1;
        int dice2;
        int consecutiveDouble;
    public :
        Dice();
        void roll();
        void setValues(int d1, int d2);
        int getTotal();
        int getDie1() const;
        int getDie2() const;
        bool checkDouble();
};