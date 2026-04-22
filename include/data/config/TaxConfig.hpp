#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class TaxConfig : public ConfigRecord {
public:
    int pphFlat = 150;
    int pphPercentage = 10;
    int pbmFlat = 200;

    TaxConfig();
    explicit TaxConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
