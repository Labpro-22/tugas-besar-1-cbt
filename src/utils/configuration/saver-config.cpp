#include "saver-config.hpp"
#include <iostream>

using namespace std;

void SaverConfig::savePropertyConfig(const string& filename, const unordered_map<int, PropertyConfig>& propertyConfigs) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    file << "ID KODE NAMA JENIS WARNA HARGA_LAHAN NILAI_GADAI UPG_RUMAH UPG_HT RENT_L0 RENT_L1 RENT_L2 RENT_L3 RENT_L4 RENT_L5" << endl;
    for (const auto& pair : propertyConfigs) {
        const PropertyConfig& config = pair.second;
        file << config.getID() << " " 
        << config.getCode() << " "
        << config.getName() << " "
        << config.getType() << " "
        << config.getColor() << " "
        << config.getLandPrice() << " "
        << config.getMortgageValue() << " "
        << config.getHouseCost() << " "
        << config.getHotelCost() << " ";
        for (int i = 0; i < 6; ++i) {
            file << config.getRent()[i] << " ";
        }
        file << endl;
    }
}

void SaverConfig::saveRailroadRentConfig(const string& filename, const vector<int>& railroadRentConfigs) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    file << "JUMLAH_RAILROAD BIAYA_SEWA" << endl;
    int index = 1;
    for (const int value : railroadRentConfigs) {
        file << index++ << " " << value << endl;
    }
}

void SaverConfig::saveUtilityRentConfig(const string& filename, const vector<int>& utilityRentConfigs) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    file << "JUMLAH_UTILITY FAKTOR_PENGALI" << endl;
    int index = 1;
    for (const int value : utilityRentConfigs) {
        file << index++ << " " << value << endl;
    }
}

void SaverConfig::saveTaxConfig(const string& filename, const TaxConfig& taxConfig) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    file << "PPH_FLAT PPH_PERSENTASE PBM_FLAT" << endl;
    file << taxConfig.getPPHFlat() << " "
         << taxConfig.getPPHPercent() << " "
         << taxConfig.getPBMFlat() << endl;
}

void SaverConfig::saveSpecialConfig(const string& filename, const SpecialConfig& specialConfig) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    file << "GO_SALARY JAIL_FINE" << endl;
    file << specialConfig.getGoSalary() << " "
         << specialConfig.getJailFine() << endl;
}

void SaverConfig::saveMiscConfig(const string& filename, const MiscConfig& miscConfig) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    file << "MAX_TURN SALDO_AWAL" << endl;
    file << miscConfig.getMaxTurn() << " "
         << miscConfig.getInitialBalance() << endl;
}

