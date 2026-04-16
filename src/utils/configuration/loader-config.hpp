#ifndef LOADER_CONFIG_HPP
#define LOADER_CONFIG_HPP
#include "property-config.hpp"
#include "special-config.hpp"
#include "tax-config.hpp"
#include "misc-config.hpp"

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class LoaderConfig{
    public:
        static void loadPropertyConfig(const string& filename, unordered_map<int, PropertyConfig>& propertyConfigs);
        static void loadRailroadRentConfig(const string& filename, vector<int>& railroadRentConfigs);
        static void loadUtilityRentConfig(const string& filename, vector<int>& utilityRentConfigs);
        static void loadTaxConfig(const string& filename, TaxConfig& taxConfig);
        static void loadSpecialConfig(const string& filename, SpecialConfig& specialConfig);
        static void loadMiscConfig(const string& filename, MiscConfig& miscConfig);

    };


#endif // LOADER_CONFIG_HPP