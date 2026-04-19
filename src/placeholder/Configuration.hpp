#pragma once

#include <map>
#include <string>
#include <vector>

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
    int id = 0;
    std::string code;
    std::string name;
    std::string propertyType;
    std::string colorGroup;
    int buyPrice = 0;
    int mortgageValue = 0;
    int houseUpgradeCost = 0;
    int hotelUpgradeCost = 0;
    std::vector<int> rentLevels;
};

struct SpecialConfig {
    int goSalary = 200;
    int jailFine = 50;
};

struct MiscConfig {
    int maxTurn = 15;
    int startingCash = 1000;
};

class Configuration {
private:
    std::string configDir;
    std::vector<BoardTileConfig> boardLayout;
    std::map<std::string, PropertyConfig> propertyConfigs;
    std::map<int, int> railroadRentTable;
    std::map<int, int> utilityMultiplierTable;
    TaxConfig taxConfig;
    SpecialConfig specialConfig;
    MiscConfig miscConfig;
    bool loaded;

    bool loadProperties();
    bool loadRailroadConfig();
    bool loadUtilityConfig();
    bool loadTaxConfig();
    bool loadSpecialConfig();
    bool loadMiscConfig();
    bool loadBoardLayout();
    bool validateBoardLayout() const;

public:
    Configuration();
    explicit Configuration(const std::string& dir);

    bool loadAllConfigs();
    bool isConfigLoaded() const;

    const std::string& getConfigDir() const;
    const std::vector<BoardTileConfig>& getBoardLayout() const;
    const std::map<int, int>& getRailroadRentTable() const;
    const std::map<int, int>& getUtilityMultiplierTable() const;

    PropertyConfig* getPropertyConfig(const std::string& code);
    const PropertyConfig* getPropertyConfig(const std::string& code) const;
    std::vector<PropertyConfig> getAllPropertyConfigs() const;

    const TaxConfig& getTaxConfig() const;
    const SpecialConfig& getSpecialConfig() const;
    const MiscConfig& getMiscConfig() const;

    int getGoSalary() const;
    int getJailFine() const;
    int getMaxTurn() const;
    int getStartingCash() const;

    void reset();
};
