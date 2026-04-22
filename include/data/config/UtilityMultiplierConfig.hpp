#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class UtilityMultiplierConfig : public ConfigRecord {
public:
    int count = 0;
    int multiplier = 0;

    UtilityMultiplierConfig();
    UtilityMultiplierConfig(int c, int m);
    explicit UtilityMultiplierConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
