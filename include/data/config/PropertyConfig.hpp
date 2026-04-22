#pragma once

#include "data/config/ConfigRecord.hpp"

#include <string>
#include <vector>

class PropertyConfig : public ConfigRecord {
public:
    int id = 0;
    std::string code;
    std::string name;
    std::string type;
    std::string colorGroup;
    int price = 0;
    int mortgageValue = 0;
    int housePrice = 0;
    int hotelPrice = 0;
    std::vector<int> rentLevels;
    int railroadRent = 0;
    int utilityMultiplier = 0;

    PropertyConfig();
    explicit PropertyConfig(const std::string& line);

    void loadFromLine(const std::string& line) override;
    void validate() const override;
    std::string toString() const override;
};
