#include "data/config/PropertyConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

#include <algorithm>
#include <sstream>

PropertyConfig::PropertyConfig() = default;

PropertyConfig::PropertyConfig(const std::string& line) {
    loadFromLine(line);
}

void PropertyConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 7) {
            throw InvalidConfigurationFormatException(
                "property config",
                "Baris properti harus memiliki minimal 7 kolom.");
        }

        PropertyConfig parsed;
        parsed.code = toUpper(tokens[1]);
        parsed.name = decodeName(tokens[2]);
        parsed.type = toUpper(tokens[3]);
        parsed.colorGroup = toUpper(tokens[4]);

        parsed.id = parseInt(tokens[0], "propertyId");
        parsed.price = parseInt(tokens[5], "price");
        parsed.mortgageValue = parseInt(tokens[6], "mortgageValue");

        if (parsed.type == "STREET") {
            if (tokens.size() < 15) {
                throw InvalidConfigurationFormatException(
                    "property config",
                    "Street harus memiliki biaya bangunan dan minimal 6 level rent.");
            }

            parsed.housePrice = parseInt(tokens[7], "housePrice");
            parsed.hotelPrice = parseInt(tokens[8], "hotelPrice");
            for (std::size_t i = 9; i < tokens.size(); ++i) {
                parsed.rentLevels.push_back(
                    parseInt(tokens[i], "rentLevel"));
            }
        } else if (parsed.type == "RAILROAD" && tokens.size() > 7) {
            parsed.railroadRent = parseInt(tokens[7], "railroadRent");
        } else if (parsed.type == "UTILITY" && tokens.size() > 7) {
            parsed.utilityMultiplier = parseInt(tokens[7], "utilityMultiplier");
        }

        parsed.validate();

        *this = parsed;
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("property config",
                                                  error.what());
    }
}

void PropertyConfig::validate() const {
    const std::string normalizedType = toUpper(type);
    if (id <= 0 || code.empty() || name.empty() ||
        (normalizedType != "STREET" && normalizedType != "RAILROAD" &&
         normalizedType != "UTILITY")) {
        throw InvalidConfigurationFormatException(
            "property config",
            "ID, kode, nama, atau tipe properti tidak valid.");
    }
    if (price <= 0 || mortgageValue < 0) {
        throw InvalidConfigurationFormatException(
            "property config",
            "Harga harus positif dan nilai gadai tidak boleh negatif.");
    }
    if (normalizedType == "STREET") {
        if (colorGroup.empty() || housePrice <= 0 || hotelPrice <= 0) {
            throw InvalidConfigurationFormatException(
                "property config",
                "Street harus memiliki warna dan biaya bangunan positif.");
        }
        if (rentLevels.size() < 6) {
            throw InvalidConfigurationFormatException(
                "property config", "Street harus memiliki minimal 6 level rent.");
        }
        if (!std::all_of(rentLevels.begin(), rentLevels.end(),
                         [](int rent) { return rent >= 0; })) {
            throw InvalidConfigurationFormatException(
                "property config", "Rent level tidak boleh negatif.");
        }
    }
}

std::string PropertyConfig::toString() const {
    std::ostringstream oss;
    oss << id << ' ' << code << ' ' << name << ' ' << type << ' '
        << colorGroup << ' ' << price << ' ' << mortgageValue;
    if (type == "STREET") {
        oss << ' ' << housePrice << ' ' << hotelPrice << ' '
            << joinInts(rentLevels);
    } else if (type == "RAILROAD" && railroadRent > 0) {
        oss << ' ' << railroadRent;
    } else if (type == "UTILITY" && utilityMultiplier > 0) {
        oss << ' ' << utilityMultiplier;
    }
    return oss.str();
}
