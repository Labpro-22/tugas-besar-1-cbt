#ifndef MISC_CONFIG_CPP
#define MISC_CONFIG_CPP

class MiscConfig {
    private:
        int maxTurn;
        int initialBalance;
    public:
        MiscConfig();
        int getMaxTurn() const;
        int getInitialBalance() const;
        void setMiscAmount(int maxTurn, int initialBalance);
};


#endif // MISC_CONFIG_CPP