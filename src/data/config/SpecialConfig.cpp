#include "data/config/SpecialConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

SpecialConfig::SpecialConfig() = default;

SpecialConfig::SpecialConfig(const std::string& line) {
    loadFromLine(line);
}

void SpecialConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2) {
            throw InvalidConfigurationFormatException(
                "special config", "Baris spesial harus berisi goSalary dan jailFine.");
        }

        goSalary = parseInt(tokens[0], "goSalary");
        jailFine = parseInt(tokens[1], "jailFine");

        validate();
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("special config", error.what());
    }
}

void SpecialConfig::validate() const {
    if (goSalary < 0 || jailFine < 0) {
        throw InvalidConfigurationFormatException(
            "special config", "Go salary dan denda penjara tidak boleh negatif.");
    }
}

std::string SpecialConfig::toString() const {
    return formatPair(goSalary, jailFine);
}
