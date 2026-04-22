#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>

class RailroadRentConfig : public ConfigRecord {
public:
    int count = 0;
    int rentAmount = 0;

    RailroadRentConfig();
    RailroadRentConfig(int c, int r);
    explicit RailroadRentConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
