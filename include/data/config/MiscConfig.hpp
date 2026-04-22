#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class MiscConfig : public ConfigRecord {
public:
    int maxTurn = 15;
    int startingCash = 1000;

    MiscConfig();
    explicit MiscConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
