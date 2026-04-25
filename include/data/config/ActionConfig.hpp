#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class ActionConfig : public ConfigRecord {
public:
    int id = 0;
    std::string code;
    std::string name;
    std::string tileType;
    std::string color;

    ActionConfig();
    explicit ActionConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
