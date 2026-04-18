#pragma once
#include <map>
#include <string>
#include <vector>

// Placeholder structs for Board initialization
struct BoardTileConfig {
    std::string code;
    std::string name;
    std::string type;
    std::string propertyCode;
};

struct TaxConfig {
    int pphFlat = 150;
    int pphPercentage = 10;
    int pbmFlat = 200;
};

struct PropertyConfig {
    std::string name;
    int buyPrice = 0;
    int mortgageValue = 0;
    int propertyDetail = 0;
    std::string propertyType;
    std::string type;
};

struct SpecialConfig {
    int goSalary = 200;
    int jailFine = 50;
};

struct MiscConfig {
    int maxTurn = 15;
    int startingCash = 1000;
};

// Placeholder Configuration class
class Configuration {
private:
    std::vector<BoardTileConfig> boardLayout;
    std::map<std::string, PropertyConfig> propertyConfigs;
    TaxConfig taxConfig;
    SpecialConfig specialConfig;
    MiscConfig miscConfig;
    bool loaded = false;

public:
    Configuration() = default;
    Configuration(const std::string & /*dir*/) {}

    bool loadAllConfigs() { return false; }
    bool isConfigLoaded() const { return loaded; }

    const std::vector<BoardTileConfig> &getBoardLayout() const {
        return boardLayout;
    }

    PropertyConfig *getPropertyConfig(const std::string &code) {
        auto it = propertyConfigs.find(code);
        if (it != propertyConfigs.end())
            return &it->second;
        return nullptr;
    }

    std::vector<PropertyConfig> getAllPropertyConfigs() const {
        std::vector<PropertyConfig> result;
        for (auto &p : propertyConfigs)
            result.push_back(p.second);
        return result;
    }

    const TaxConfig &getTaxConfig() const { return taxConfig; }
    const SpecialConfig &getSpecialConfig() const { return specialConfig; }
    const MiscConfig &getMiscConfig() const { return miscConfig; }

    int getGoSalary() const { return specialConfig.goSalary; }
    int getJailFine() const { return specialConfig.jailFine; }
    int getMaxTurn() const { return miscConfig.maxTurn; }
    int getStartingCash() const { return miscConfig.startingCash; }

    void reset() {
        boardLayout.clear();
        propertyConfigs.clear();
        loaded = false;
    }
};
