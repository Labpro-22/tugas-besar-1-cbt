#pragma once

#include "data/config/ActionConfig.hpp"
#include "data/config/BoardTileConfig.hpp"
#include "data/config/MiscConfig.hpp"
#include "data/config/PropertyConfig.hpp"
#include "data/config/RailroadRentConfig.hpp"
#include "data/config/SpecialConfig.hpp"
#include "data/config/TaxConfig.hpp"
#include "data/config/UtilityMultiplierConfig.hpp"

#include <map>
#include <string>
#include <vector>

class Configuration {
private:
    std::string configDir;
    std::vector<BoardTileConfig> boardLayout;
    std::map<int, ActionConfig> actionConfigs;
    std::map<std::string, PropertyConfig> propertyConfigs;
    std::map<int, RailroadRentConfig> railroadRentTable;
    std::map<int, UtilityMultiplierConfig> utilityMultiplierTable;
    TaxConfig taxConfig;
    SpecialConfig specialConfig;
    MiscConfig miscConfig;
    bool loaded;
    std::string lastError;

    void generateBoardLayout();
    void validateBoardLayout();
    void validatePropertyConfig(const PropertyConfig& config) const;
    void setLastError(const std::string& message);
    static std::string toUpper(std::string text);
    static std::string toLower(std::string text);
    static char firstNonWhitespaceChar(const std::string& line);

public:
    Configuration();
    explicit Configuration(const std::string& dir);

    void loadAllConfigs();
    void loadActionConfigs(const std::string& path);
    void loadPropertyConfigs(const std::string& path);
    void loadRailroadConfigs(const std::string& path);
    void loadUtilityConfigs(const std::string& path);
    void loadTaxConfig(const std::string& path);
    void loadSpecialConfig(const std::string& path);
    void loadMiscConfig(const std::string& path);
    bool isConfigLoaded() const;
    const std::string& getLastError() const;

    const std::string& getConfigDir() const;
    const std::vector<BoardTileConfig>& getBoardLayout() const;
    const std::map<int, ActionConfig>& getActionConfigs() const;
    const std::map<int, RailroadRentConfig>& getRailroadRentConfigs() const;
    const std::map<int, UtilityMultiplierConfig>& getUtilityMultiplierConfigs() const;
    std::map<int, int> getRailroadRentTable() const;
    std::map<int, int> getUtilityMultiplierTable() const;

    PropertyConfig* getPropertyConfig(const std::string& code);
    const PropertyConfig* getPropertyConfig(const std::string& code) const;
    std::vector<PropertyConfig> getAllPropertyConfigs() const;
    ActionConfig* getActionConfig(int id);
    const ActionConfig* getActionConfig(int id) const;
    std::vector<ActionConfig> getAllActionConfigs() const;

    int getRailroadRent(int count) const;
    int getUtilityMultiplier(int count) const;

    TaxConfig& getTaxConfig();
    const TaxConfig& getTaxConfig() const;
    SpecialConfig& getSpecialConfig();
    const SpecialConfig& getSpecialConfig() const;
    MiscConfig& getMiscConfig();
    const MiscConfig& getMiscConfig() const;

    int getGoSalary() const;
    int getJailFine() const;
    int getMaxTurn() const;
    int getStartingCash() const;

    void reset();
};
