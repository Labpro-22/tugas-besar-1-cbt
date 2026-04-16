#ifndef TAX_CONFIG_HPP
#define TAX_CONFIG_HPP

class TaxConfig {
    private:
        int pph_flat;
        int pph_percent;
        int pbm_flat;

    public:
        TaxConfig();
        int getPPHFlat() const;
        int getPPHPercent() const;
        int getPBMFlat() const;
        void setTaxAmount(int pph_flat, int pph_percent, int pbm_flat);
};


#endif // TAX_CONFIG_HPP