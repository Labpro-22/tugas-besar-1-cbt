#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <unordered_map>
#include <string>

#include "loader-config.hpp"

// Stub to avoid error
// #define PropertyConfig int
#define RailroadRentConfig int
#define UtilityConfig int
// #define TaxConfig int
// #define SpecialConfig int
// #define MiscConfig int

#include <vector>
using namespace std;

// Configuration class, holds all the configuration data for the game.
// NOTE: config loader in ConfigurationLoader class.
class Configuration {
    private:
        /* ----------------------------- Plural configs ----------------------------- */
        //use unordered_map for O(1) access time and non sequential keys
        //int key is the property ID, value is the property config
        unordered_map<int, PropertyConfig> propertyConfigs;
        //use vector for sequential keys and O(1) access time, not using
        //class because will be overengineered.
        //int key is the number of railroads owned, value is the railroad rent multiplier
        vector<int> railroadRentConfigs;
        //use vector for sequential keys and O(1) access time, not using
        //class because will be overengineered.
        //int key is the number of railroads owned, value is the utility rent multiplier
        vector<int> utilityRentConfigs;

        /* ------------------------------ Unary configs ----------------------------- */
        TaxConfig taxConfig;
        SpecialConfig specialConfig;
        MiscConfig miscConfig;

        /* -------------------------------- Utilities ------------------------------- */
        string configDir;
        bool isLoaded;

    public:
        Configuration();
        PropertyConfig getProperty(int key) const;
        void setProperty(int key, int value);
        RailroadRentConfig getRailroadRent(int key) const;
        void setRailroadRent(int key, int value);
        UtilityConfig getUtilityRent(int key) const;
        void setUtilityRent(int key, int value);

        TaxConfig getTax() const;
        void setTax(int pph_flat, int pph_percent, int pbm_flat);

        SpecialConfig getSpecial() const;
        void setSpecial(int goSalary, int jailFine);

        MiscConfig getMisc() const;
        void setMisc(int maxTurn, int initialBalance);

        void loadConfig(const string& filename);
        void saveConfig(const string& filename) const;
};


#endif // CONFIGURATION_H