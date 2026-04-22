#include "data/config/MiscConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

MiscConfig::MiscConfig() = default;

MiscConfig::MiscConfig(const std::string& line) {
    loadFromLine(line);
}

void MiscConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2) {
            throw InvalidConfigurationFormatException(
                "misc config", "Baris misc harus berisi maxTurn dan startingCash.");
        }

        maxTurn = parseInt(tokens[0], "maxTurn");
        startingCash = parseInt(tokens[1], "startingCash");

        validate();
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("misc config", error.what());
    }
}

void MiscConfig::validate() const {
    if (maxTurn <= 0) {
        throw InvalidConfigurationFormatException(
            "misc config", "Max turn harus lebih besar dari 0.");
    }
    if (startingCash < 0) {
        throw InvalidConfigurationFormatException(
            "misc config", "Saldo awal tidak boleh negatif.");
    }
}

std::string MiscConfig::toString() const {
    return formatPair(maxTurn, startingCash);
}
