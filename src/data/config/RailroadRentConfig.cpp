#include "data/config/RailroadRentConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

RailroadRentConfig::RailroadRentConfig() = default;

RailroadRentConfig::RailroadRentConfig(int c, int r)
    : count(c), rentAmount(r) {}

RailroadRentConfig::RailroadRentConfig(const std::string& line) {
    loadFromLine(line);
}

void RailroadRentConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2) {
            throw InvalidConfigurationFormatException(
                "railroad config", "Baris railroad harus berisi jumlah dan biaya sewa.");
        }

        count = parseInt(tokens[0], "railroadCount");
        rentAmount = parseInt(tokens[1], "rentAmount");

        validate();
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("railroad config", error.what());
    }
}

void RailroadRentConfig::validate() const {
    if (count <= 0 || rentAmount < 0) {
        throw InvalidConfigurationFormatException(
            "railroad config", "Jumlah railroad harus positif dan biaya sewa tidak boleh negatif.");
    }
}

std::string RailroadRentConfig::toString() const {
    return formatPair(count, rentAmount);
}
