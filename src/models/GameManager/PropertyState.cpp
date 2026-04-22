#include "../../../include/models/GameManager/PropertyState.hpp"


PropertyState::PropertyState(int fmult, int fdur, int buildingLevel) : fmult(fmult), fdur(fdur), buildingLevel(buildingLevel) {};
int PropertyState::getFmult() const{
    return fmult;
}
int PropertyState::getFdur() const{
    return fdur;
}
int PropertyState::getBuildLevel() const{
    return buildingLevel;
}