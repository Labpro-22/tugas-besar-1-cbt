#include "loader-config.hpp"
#include <iostream>
#include <fstream>
using namespace std;

void LoaderConfig::loadPropertyConfig(const string& filename, unordered_map<int, PropertyConfig>& propertyConfigs) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string header;
    getline(file, header);
    
    propertyConfigs.clear();
    PropertyConfig curr;

    while (file >> curr) {
        propertyConfigs[curr.getID()] = curr;
    }
}

void LoaderConfig::loadRailroadRentConfig(const string& filename, vector<int>& railroadRentConfigs) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string header;
    getline(file, header);

    railroadRentConfigs.clear();
    int rentValue;
    while (file >> rentValue) {
        railroadRentConfigs.push_back(rentValue);
    }
}

void LoaderConfig::loadUtilityRentConfig(const string& filename, vector<int>& utilityRentConfigs) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string header;
    getline(file, header);

    utilityRentConfigs.clear();
    int rentValue;
    while (file >> rentValue) {
        utilityRentConfigs.push_back(rentValue);
    }
}

void LoaderConfig::loadTaxConfig(const string& filename, TaxConfig& taxConfig) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string header;
    getline(file, header);

    int pphFlat, pphPercent, pbmFlat;
    if (!(file >> pphFlat >> pphPercent >> pbmFlat)) {
        cerr << "Invalid tax config format in file: " << filename << endl;
        return;
    }

    taxConfig.setTaxAmount(pphFlat, pphPercent, pbmFlat);
}

void LoaderConfig::loadSpecialConfig(const string& filename, SpecialConfig& specialConfig) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string header;
    getline(file, header);

    int goSalary, jailFine;
    if (!(file >> goSalary >> jailFine)) {
        cerr << "Invalid special config format in file: " << filename << endl;
        return;
    }

    specialConfig.setSpecialAmount(goSalary, jailFine);
}

void LoaderConfig::loadMiscConfig(const string& filename, MiscConfig& miscConfig) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string header;
    getline(file, header);

    int maxTurn, initialBalance;
    if (!(file >> maxTurn >> initialBalance)) {
        cerr << "Invalid misc config format in file: " << filename << endl;
        return;
    }

    miscConfig.setMiscAmount(maxTurn, initialBalance);
}
