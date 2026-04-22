#include "models/GameManager/Dice.hpp"

Dice::Dice() : dice1(0), dice2(0), consecutiveDouble(0) {}
int Dice::roll() {
    random_device rd;
    mt19937 gen(rd());
    int min = 1;
    int max = 6;

    uniform_int_distribution<> dist(min, max);

    dice1 = dist(gen);
    dice2 = dist(gen);

    if (dice1 == dice2) {
        consecutiveDouble++;
    } else {
        consecutiveDouble = 0;
    }

    return dice1 + dice2;
}

void Dice::setValues(int d1, int d2) {
    dice1 = d1;
    dice2 = d2;

    if (dice1 == dice2) {
        consecutiveDouble++;
    } else {
        consecutiveDouble = 0;
    }
}

int Dice::getTotal() { return dice1 + dice2; }
int Dice::getDie1() const { return dice1; }
int Dice::getDie2() const { return dice2; }
bool Dice::checkDouble() { return (dice1 == dice2); }

int Dice::getConsecutiveDoubles() const { return consecutiveDouble; }

void Dice::resetConsecutiveDoubles() { consecutiveDouble = 0; }

bool Dice::hasThreeConsecutiveDoubles() const { return consecutiveDouble >= 3; }