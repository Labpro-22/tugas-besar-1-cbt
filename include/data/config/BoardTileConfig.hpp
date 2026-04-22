#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class BoardTileConfig : public ConfigRecord {
public:
    std::string code;
    std::string name;
    std::string type;
    std::string propertyCode;

    BoardTileConfig();
    explicit BoardTileConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
