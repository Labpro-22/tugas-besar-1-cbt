#ifndef SAVER_CONFIG_HPP
#define SAVER_CONFIG_HPP

#include <fstream>
#include <string>
#include <unordered_map>

#include "configuration.hpp"

using namespace std;

class SaverConfig {
    public:
        static void savePropertyConfig(const string& filename, const unordered_map<int, PropertyConfig>& propertyConfigs);
        static void saveRailroadRentConfig(const string& filename, const vector<int>& railroadRentConfigs);
        static void saveUtilityRentConfig(const string& filename, const vector<int>& utilityRentConfigs);
        static void saveTaxConfig(const string& filename, const TaxConfig& taxConfig);
        static void saveSpecialConfig(const string& filename, const SpecialConfig& specialConfig);
        static void saveMiscConfig(const string& filename, const MiscConfig& miscConfig);
};

#endif // SAVER_CONFIG_HPP