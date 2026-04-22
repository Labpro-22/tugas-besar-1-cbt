#include "data/config/UtilityMultiplierConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

UtilityMultiplierConfig::UtilityMultiplierConfig() = default;

UtilityMultiplierConfig::UtilityMultiplierConfig(int c, int m)
    : count(c), multiplier(m) {}

UtilityMultiplierConfig::UtilityMultiplierConfig(const std::string& line) {
    loadFromLine(line);
}

void UtilityMultiplierConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2) {
            throw InvalidConfigurationFormatException(
                "utility config", "Baris utility harus berisi jumlah dan multiplier.");
        }

        count = parseInt(tokens[0], "utilityCount");
        multiplier = parseInt(tokens[1], "multiplier");

        validate();
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("utility config", error.what());
    }
}

void UtilityMultiplierConfig::validate() const {
    if (count <= 0 || multiplier < 0) {
        throw InvalidConfigurationFormatException(
            "utility config", "Jumlah utility harus positif dan multiplier tidak boleh negatif.");
    }
}

std::string UtilityMultiplierConfig::toString() const {
    return formatPair(count, multiplier);
}
