#pragma once

#include "data/config/ActionConfig.hpp"
#include "data/config/ConfigRecord.hpp"
#include "data/config/PropertyConfig.hpp"

#include <map>
#include <string>
#include <vector>

class BoardTileConfig : public ConfigRecord {
public:
    std::string code;
    std::string name;
    std::string type;
    std::string propertyCode;

    BoardTileConfig();
    explicit BoardTileConfig(const std::string& line);
    BoardTileConfig(const ActionConfig& action);
    BoardTileConfig(const PropertyConfig& property);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;

    static BoardTileConfig fromActionConfig(const ActionConfig& action);
    static BoardTileConfig fromPropertyConfig(const PropertyConfig& property);
    static std::vector<BoardTileConfig> loadFromConfigs(
        const std::map<int, ActionConfig>& actions,
        const std::vector<PropertyConfig>& properties);
};
