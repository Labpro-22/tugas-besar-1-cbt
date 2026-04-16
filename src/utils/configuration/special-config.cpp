#include "special-config.hpp"

SpecialConfig::SpecialConfig() : goSalary(0), jailFine(0) {};
int SpecialConfig::getGoSalary() const {
    return goSalary;
}
int SpecialConfig::getJailFine() const {
    return jailFine;
}
void SpecialConfig::setSpecialAmount(int goSalary, int jailFine) {
    this->goSalary = goSalary;
    this->jailFine = jailFine;
}
