#include "tax-config.hpp"

TaxConfig::TaxConfig() : pph_flat(0), pph_percent(0), pbm_flat(0) {};
int TaxConfig::getPPHFlat() const {
    return pph_flat;
}
int TaxConfig::getPPHPercent() const {
    return pph_percent;
}
int TaxConfig::getPBMFlat() const {
    return pbm_flat;
}
void TaxConfig::setTaxAmount(int pph_flat, int pph_percent, int pbm_flat) {
    this->pph_flat = pph_flat;
    this->pph_percent = pph_percent;
    this->pbm_flat = pbm_flat;
}
