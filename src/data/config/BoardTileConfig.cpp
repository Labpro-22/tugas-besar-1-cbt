#include "data/config/BoardTileConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

#include <map>
#include <sstream>

BoardTileConfig::BoardTileConfig() = default;

BoardTileConfig::BoardTileConfig(const std::string& line) {
    loadFromLine(line);
}

BoardTileConfig::BoardTileConfig(const ActionConfig& action)
    : BoardTileConfig(fromActionConfig(action)) {}

BoardTileConfig::BoardTileConfig(const PropertyConfig& property)
    : BoardTileConfig(fromPropertyConfig(property)) {}

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

BoardTileConfig BoardTileConfig::fromActionConfig(const ActionConfig& action) {
    BoardTileConfig tile;
    tile.code = toUpper(action.code);
    tile.name = action.name;
    tile.propertyCode = "";

    const std::string actionType = toUpper(action.tileType);
    const std::string actionCode = toUpper(action.code);

    if (actionType == "SPESIAL") {
        if (actionCode == "GO") {
            tile.type = "GO";
        } else if (actionCode == "PEN") {
            tile.type = "JAIL";
        } else if (actionCode == "BBP") {
            tile.type = "FREE_PARKING";
        } else if (actionCode == "PPJ") {
            tile.type = "GO_TO_JAIL";
        } else {
            throw InvalidConfigurationFormatException(
                "action config", "Kode petak spesial tidak dikenali.");
        }
    } else if (actionType == "KARTU") {
        if (actionCode == "DNU") {
            tile.type = "COMMUNITY_CHEST";
        } else if (actionCode == "KSP") {
            tile.type = "CHANCE";
        } else {
            throw InvalidConfigurationFormatException(
                "action config", "Kode petak kartu tidak dikenali.");
        }
    } else if (actionType == "PAJAK") {
        if (actionCode == "PPH") {
            tile.type = "PPH";
        } else if (actionCode == "PBM") {
            tile.type = "PBM";
        } else {
            throw InvalidConfigurationFormatException(
                "action config", "Kode petak pajak tidak dikenali.");
        }
    } else if (actionType == "FESTIVAL") {
        tile.type = "FESTIVAL";
    } else {
        throw InvalidConfigurationFormatException(
            "action config", "Jenis petak aksi tidak dikenali.");
    }

    tile.validate();
    return tile;
}

BoardTileConfig BoardTileConfig::fromPropertyConfig(
    const PropertyConfig& property) {
    BoardTileConfig tile;
    tile.code = toUpper(property.code);
    tile.name = property.name;
    tile.type = "PROPERTY";
    tile.propertyCode = toUpper(property.code);
    tile.validate();
    return tile;
}

// Static method to generate vector of board tiles from action and property configs
std::vector<BoardTileConfig> BoardTileConfig::loadFromConfigs(
    const std::map<int, ActionConfig>& actions,
    const std::vector<PropertyConfig>& properties) {
    std::map<int, BoardTileConfig> generatedLayout;

    for (const auto& [id, action] : actions) {
        auto inserted = generatedLayout.emplace(id, fromActionConfig(action));
        if (!inserted.second) {
            throw InvalidBoardConfigurationException(
                "ID petak " + std::to_string(id) +
                " digunakan lebih dari satu kali.");
        }
    }

    for (const PropertyConfig& property : properties) {
        auto inserted =
            generatedLayout.emplace(property.id, fromPropertyConfig(property));
        if (!inserted.second) {
            throw InvalidBoardConfigurationException(
                "ID petak " + std::to_string(property.id) +
                " digunakan lebih dari satu kali.");
        }
    }

    std::vector<BoardTileConfig> boardLayout;
    int expectedId = 1;
    boardLayout.reserve(generatedLayout.size());
    for (const auto& [id, tile] : generatedLayout) {
        if (id != expectedId) {
            throw InvalidBoardConfigurationException(
                "ID petak harus berurutan mulai dari 1. ID " +
                std::to_string(expectedId) + " tidak ditemukan.");
        }
        boardLayout.push_back(tile);
        expectedId++;
    }

    if (boardLayout.empty()) {
        throw InvalidBoardConfigurationException(
            "Konfigurasi aksi dan properti tidak menghasilkan petak.");
    }

    return boardLayout;
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
