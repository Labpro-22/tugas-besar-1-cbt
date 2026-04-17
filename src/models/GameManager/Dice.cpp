#include "Dice.hpp"


Dice::Dice() : dice1(0), dice2(0), consecutiveDouble(0){}
int Dice::roll(){
    random_device rd;
    mt19937 gen(rd());
    int min = 1;
    int max = 6;

    uniform_int_distribution<> dist(min, max);

    dice1 = dist(gen);
    dice2 = dist(gen);
    return dice1 + dice2;
}
int Dice::getTotal(){
    return dice1+dice2;
}
int Dice::getDie1() const{
    return dice1;
}
int Dice::getDie2() const{
    return dice2;
}
bool Dice::checkDouble(){
    if(dice1 == dice2){
        consecutiveDouble += 1;
        return true;
    }
    return (dice1 == dice2);
}