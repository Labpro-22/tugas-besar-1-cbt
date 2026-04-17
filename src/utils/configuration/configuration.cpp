#include "configuration.hpp"
#include "loader-config.hpp"
#include <iostream>

using namespace std;

Configuration::Configuration() : configDir(""), isLoaded(false) {
}

// Property configuration methods
PropertyConfig Configuration::getProperty(int key) const {
    auto it = propertyConfigs.find(key);
    if (it != propertyConfigs.end()) {
        return it->second;
    }
    cerr << "Warning: Property with key " << key << " not found." << endl;
    return PropertyConfig(); // Return default PropertyConfig
}

void Configuration::setProperty(int key, int value) {
    if (propertyConfigs.find(key) != propertyConfigs.end()) {
        propertyConfigs[key].setPropertyAmount(value, value);
    } else {
        cerr << "Warning: Property with key " << key << " not found." << endl;
    }
}

// Railroad rent configuration methods
RailroadRentConfig Configuration::getRailroadRent(int key) const {
    if (key >= 0 && key < static_cast<int>(railroadRentConfigs.size())) {
        return railroadRentConfigs[key];
    }
    cerr << "Warning: Railroad rent config at index " << key << " not found." << endl;
    return 0;
}

void Configuration::setRailroadRent(int key, int value) {
    if (key >= 0 && key < static_cast<int>(railroadRentConfigs.size())) {
        railroadRentConfigs[key] = value;
    } else {
        cerr << "Warning: Railroad rent config at index " << key << " not found." << endl;
    }
}

// Utility rent configuration methods
UtilityConfig Configuration::getUtilityRent(int key) const {
    if (key >= 0 && key < static_cast<int>(utilityRentConfigs.size())) {
        return utilityRentConfigs[key];
    }
    cerr << "Warning: Utility rent config at index " << key << " not found." << endl;
    return 0;
}

void Configuration::setUtilityRent(int key, int value) {
    if (key >= 0 && key < static_cast<int>(utilityRentConfigs.size())) {
        utilityRentConfigs[key] = value;
    } else {
        cerr << "Warning: Utility rent config at index " << key << " not found." << endl;
    }
}

// Tax configuration methods
TaxConfig Configuration::getTax() const {
    return taxConfig;
}

void Configuration::setTax(int pph_flat, int pph_percent, int pbm_flat) {
    taxConfig.setTaxAmount(pph_flat, pph_percent, pbm_flat);
}

// Special configuration methods
SpecialConfig Configuration::getSpecial() const {
    return specialConfig;
}

void Configuration::setSpecial(int goSalary, int jailFine) {
    specialConfig.setSpecialAmount(goSalary, jailFine);
}

// Misc configuration methods
MiscConfig Configuration::getMisc() const {
    return miscConfig;
}

void Configuration::setMisc(int maxTurn, int initialBalance) {
    miscConfig.setMiscAmount(maxTurn, initialBalance);
}

// Configuration loader and saver methods
void Configuration::loadConfig(const string& filename) {
    configDir = filename;
        
    string baseDir = configDir;
    if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\') {
        baseDir += "/";
    }
    
    // Load property configurations
    string propertyFile = baseDir + "properties.txt";
    LoaderConfig::loadPropertyConfig(propertyFile, propertyConfigs);
    
    // Load railroad rent configurations
    string railroadFile = baseDir + "railroad.txt";
    LoaderConfig::loadRailroadRentConfig(railroadFile, railroadRentConfigs);
    
    // Load utility rent configurations
    string utilityFile = baseDir + "utility.txt";
    LoaderConfig::loadUtilityRentConfig(utilityFile, utilityRentConfigs);
    
    // Load tax configurations
    string taxFile = baseDir + "tax.txt";
    LoaderConfig::loadTaxConfig(taxFile, taxConfig);
    
    // Load special configurations
    string specialFile = baseDir + "special.txt";
    LoaderConfig::loadSpecialConfig(specialFile, specialConfig);
    
    // Load misc configurations
    string miscFile = baseDir + "misc.txt";
    LoaderConfig::loadMiscConfig(miscFile, miscConfig);
    
    isLoaded = true;
    cout << "Configuration loaded from: " << configDir << endl;
}

void Configuration::saveConfig(const string& filename) const {
    // TODO: Implement save configuration to file
    cerr << "Warning: saveConfig() is not yet implemented." << endl;
}
