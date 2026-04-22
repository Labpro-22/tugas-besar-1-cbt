#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class SpecialConfig : public ConfigRecord {
public:
    int goSalary = 200;
    int jailFine = 50;

    SpecialConfig();
    explicit SpecialConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
