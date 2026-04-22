#include "data/config/BoardTileConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

#include <sstream>

BoardTileConfig::BoardTileConfig() = default;

BoardTileConfig::BoardTileConfig(const std::string& line) {
    loadFromLine(line);
}

void BoardTileConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 3) {
            throw InvalidConfigurationFormatException(
                "board config", "Baris papan harus memiliki kode, nama, dan tipe.");
        }

        code = toUpper(tokens[0]);
        name = decodeName(tokens[1]);
        type = toLower(tokens[2]);
        propertyCode = tokens.size() > 3 ? toUpper(tokens[3]) : "";

        validate();
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("board config", error.what());
    }
}

void BoardTileConfig::validate() const {
    if (code.empty() || name.empty() || type.empty()) {
        throw InvalidConfigurationFormatException(
            "board config", "Kode, nama, dan tipe petak tidak boleh kosong.");
    }
}

std::string BoardTileConfig::toString() const {
    std::ostringstream oss;
    oss << code << ' ' << name << ' ' << type;
    if (!propertyCode.empty()) {
        oss << ' ' << propertyCode;
    }
    return oss.str();
}
