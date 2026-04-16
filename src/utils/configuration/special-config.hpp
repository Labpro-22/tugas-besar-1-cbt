#ifndef SPECIAL_CONFIG_HPP
#define SPECIAL_CONFIG_HPP

class SpecialConfig {
    private:
        int goSalary;
        int jailFine;
    public:
        SpecialConfig();
        int getGoSalary() const;
        int getJailFine() const;
        void setSpecialAmount(int goSalary, int jailFine);
};

#endif //SPECIAL_CONFIG_HPP