#include "data/config/ActionConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

#include <sstream>

ActionConfig::ActionConfig() = default;

ActionConfig::ActionConfig(const std::string& line) {
    loadFromLine(line);
}

void ActionConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 5) {
            throw InvalidConfigurationFormatException(
                "action config",
                "Baris aksi harus memiliki ID, kode, nama, jenis petak, dan warna.");
        }

        ActionConfig parsed;
        parsed.id = parseInt(tokens[0], "actionId");
        parsed.code = toUpper(tokens[1]);
        parsed.name = decodeName(tokens[2]);
        parsed.tileType = toUpper(tokens[3]);
        parsed.color = toUpper(tokens[4]);

        parsed.validate();

        *this = parsed;
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("action config", error.what());
    }
}

void ActionConfig::validate() const {
    if (id <= 0 || code.empty() || name.empty() || tileType.empty() ||
        color.empty()) {
        throw InvalidConfigurationFormatException(
            "action config",
            "ID, kode, nama, jenis petak, dan warna aksi harus valid.");
    }

    if (tileType != "SPESIAL" && tileType != "KARTU" &&
        tileType != "PAJAK" && tileType != "FESTIVAL") {
        throw InvalidConfigurationFormatException(
            "action config", "Jenis petak aksi tidak dikenali.");
    }
}

std::string ActionConfig::toString() const {
    std::ostringstream oss;
    oss << id << ' ' << code << ' ' << name << ' ' << tileType << ' ' << color;
    return oss.str();
}
