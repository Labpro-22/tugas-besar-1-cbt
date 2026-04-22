#include "data/Configuration.hpp"

#include "exception/NimonspoliExceptions.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

std::filesystem::path configPath(const std::string& dir,
                                 const std::string& filename) {
    return std::filesystem::path(dir) / filename;
}

std::string filenameOf(const std::string& path) {
    const std::filesystem::path filePath(path);
    const std::string filename = filePath.filename().string();
    return filename.empty() ? path : filename;
}

}  // namespace

Configuration::Configuration() : configDir("config"), loaded(false), lastError() {}

Configuration::Configuration(const std::string& dir)
    : configDir(dir), loaded(false), lastError() {}

void Configuration::loadAllConfigs() {
    reset();

    try {
        loadPropertyConfigs(configPath(configDir, "property.txt").string());
        loadRailroadConfigs(configPath(configDir, "railroad.txt").string());
        loadUtilityConfigs(configPath(configDir, "utility.txt").string());
        loadTaxConfig(configPath(configDir, "tax.txt").string());
        loadSpecialConfig(configPath(configDir, "special.txt").string());
        loadMiscConfig(configPath(configDir, "misc.txt").string());

        const std::filesystem::path boardPath = configPath(configDir, "board.txt");
        const std::filesystem::path fallbackBoardPath =
            configPath(configDir, "board_layout.txt");
        if (std::filesystem::exists(boardPath)) {
            loadBoardLayout(boardPath.string());
        } else {
            loadBoardLayout(fallbackBoardPath.string());
        }

        validateBoardLayout();

        loaded = true;
        lastError.clear();
    } catch (const NimonspoliException& error) {
        loaded = false;
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const ConfigurationException wrapped("Kesalahan konfigurasi. " +
                                             std::string(error.what()));
        loaded = false;
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::loadPropertyConfigs(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        propertyConfigs.clear();
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#' ||
                !std::isdigit(static_cast<unsigned char>(firstChar))) {
                continue;
            }
            PropertyConfig property;
            property.loadFromLine(line);
            validatePropertyConfig(property);

            propertyConfigs[property.code] = property;
        }

        if (propertyConfigs.empty()) {
            throw InvalidConfigurationFormatException(
                filenameOf(path), "Tidak ada properti valid.");
        }

        lastError.clear();
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidConfigurationFormatException wrapped(filenameOf(path),
                                                          error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::loadRailroadConfigs(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        railroadRentTable.clear();
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#' ||
                !std::isdigit(static_cast<unsigned char>(firstChar))) {
                continue;
            }
            RailroadRentConfig rentConfig;
            rentConfig.loadFromLine(line);

            railroadRentTable[rentConfig.count] = rentConfig;
        }

        if (railroadRentTable.empty()) {
            throw InvalidConfigurationFormatException(
                filenameOf(path), "Tabel sewa railroad kosong.");
        }

        lastError.clear();
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidConfigurationFormatException wrapped(filenameOf(path),
                                                          error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::loadUtilityConfigs(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        utilityMultiplierTable.clear();
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#' ||
                !std::isdigit(static_cast<unsigned char>(firstChar))) {
                continue;
            }
            UtilityMultiplierConfig multiplierConfig;
            multiplierConfig.loadFromLine(line);

            utilityMultiplierTable[multiplierConfig.count] = multiplierConfig;
        }

        if (utilityMultiplierTable.empty()) {
            throw InvalidConfigurationFormatException(
                filenameOf(path), "Tabel multiplier utility kosong.");
        }

        lastError.clear();
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidConfigurationFormatException wrapped(filenameOf(path),
                                                          error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::loadTaxConfig(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#' ||
                !std::isdigit(static_cast<unsigned char>(firstChar))) {
                continue;
            }
            taxConfig.loadFromLine(line);

            lastError.clear();
            return;
        }

        throw InvalidConfigurationFormatException(filenameOf(path),
                                                  "Konfigurasi pajak kosong.");
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidConfigurationFormatException wrapped(filenameOf(path),
                                                          error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::loadSpecialConfig(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#' ||
                !std::isdigit(static_cast<unsigned char>(firstChar))) {
                continue;
            }
            specialConfig.loadFromLine(line);

            lastError.clear();
            return;
        }

        throw InvalidConfigurationFormatException(
            filenameOf(path), "Konfigurasi petak spesial kosong.");
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidConfigurationFormatException wrapped(filenameOf(path),
                                                          error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::loadMiscConfig(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#' ||
                !std::isdigit(static_cast<unsigned char>(firstChar))) {
                continue;
            }
            miscConfig.loadFromLine(line);

            lastError.clear();
            return;
        }

        throw InvalidConfigurationFormatException(filenameOf(path),
                                                  "Konfigurasi misc kosong.");
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidConfigurationFormatException wrapped(filenameOf(path),
                                                          error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

bool Configuration::isConfigLoaded() const { return loaded; }

const std::string& Configuration::getLastError() const { return lastError; }

const std::string& Configuration::getConfigDir() const { return configDir; }

const std::vector<BoardTileConfig>& Configuration::getBoardLayout() const {
    return boardLayout;
}

const std::map<int, RailroadRentConfig>&
Configuration::getRailroadRentConfigs() const {
    return railroadRentTable;
}

const std::map<int, UtilityMultiplierConfig>&
Configuration::getUtilityMultiplierConfigs() const {
    return utilityMultiplierTable;
}

std::map<int, int> Configuration::getRailroadRentTable() const {
    std::map<int, int> result;
    for (const auto& [count, config] : railroadRentTable) {
        result[count] = config.rentAmount;
    }
    return result;
}

std::map<int, int> Configuration::getUtilityMultiplierTable() const {
    std::map<int, int> result;
    for (const auto& [count, config] : utilityMultiplierTable) {
        result[count] = config.multiplier;
    }
    return result;
}

PropertyConfig* Configuration::getPropertyConfig(const std::string& code) {
    auto it = propertyConfigs.find(toUpper(code));
    if (it == propertyConfigs.end()) {
        return nullptr;
    }
    return &it->second;
}

const PropertyConfig* Configuration::getPropertyConfig(const std::string& code) const {
    auto it = propertyConfigs.find(toUpper(code));
    if (it == propertyConfigs.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<PropertyConfig> Configuration::getAllPropertyConfigs() const {
    std::vector<PropertyConfig> result;
    result.reserve(propertyConfigs.size());
    for (const auto& [_, property] : propertyConfigs) {
        result.push_back(property);
    }
    return result;
}

int Configuration::getRailroadRent(int count) const {
    if (railroadRentTable.empty()) {
        return 0;
    }

    auto exact = railroadRentTable.find(count);
    if (exact != railroadRentTable.end()) {
        return exact->second.rentAmount;
    }

    auto upper = railroadRentTable.upper_bound(count);
    if (upper == railroadRentTable.begin()) {
        return upper->second.rentAmount;
    }
    return std::prev(upper)->second.rentAmount;
}

int Configuration::getUtilityMultiplier(int count) const {
    if (utilityMultiplierTable.empty()) {
        return 0;
    }

    auto exact = utilityMultiplierTable.find(count);
    if (exact != utilityMultiplierTable.end()) {
        return exact->second.multiplier;
    }

    auto singleOwned = utilityMultiplierTable.find(1);
    if (singleOwned != utilityMultiplierTable.end()) {
        return singleOwned->second.multiplier;
    }
    return utilityMultiplierTable.begin()->second.multiplier;
}

TaxConfig& Configuration::getTaxConfig() { return taxConfig; }

const TaxConfig& Configuration::getTaxConfig() const { return taxConfig; }

SpecialConfig& Configuration::getSpecialConfig() { return specialConfig; }

const SpecialConfig& Configuration::getSpecialConfig() const { return specialConfig; }

MiscConfig& Configuration::getMiscConfig() { return miscConfig; }

const MiscConfig& Configuration::getMiscConfig() const { return miscConfig; }

int Configuration::getGoSalary() const { return specialConfig.goSalary; }

int Configuration::getJailFine() const { return specialConfig.jailFine; }

int Configuration::getMaxTurn() const { return miscConfig.maxTurn; }

int Configuration::getStartingCash() const { return miscConfig.startingCash; }

void Configuration::reset() {
    boardLayout.clear();
    propertyConfigs.clear();
    railroadRentTable.clear();
    utilityMultiplierTable.clear();
    taxConfig = TaxConfig();
    specialConfig = SpecialConfig();
    miscConfig = MiscConfig();
    loaded = false;
    lastError.clear();
}

void Configuration::loadBoardLayout(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw MissingConfigurationFileException(path);
        }

        boardLayout.clear();
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            const char firstChar = firstNonWhitespaceChar(line);
            if (firstChar == '\0' || firstChar == '#') {
                continue;
            }

            BoardTileConfig tile;
            tile.loadFromLine(line);

            boardLayout.push_back(tile);
        }

        if (boardLayout.empty()) {
            throw InvalidBoardConfigurationException(
                "File papan tidak berisi petak valid.");
        }

        lastError.clear();
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidBoardConfigurationException wrapped(error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::validateBoardLayout() {
    try {
        if (boardLayout.size() < 20 || boardLayout.size() > 60) {
            throw InvalidBoardConfigurationException(
                "Jumlah petak " + std::to_string(boardLayout.size()) +
                ", harus minimal 20 dan maksimal 60.");
        }

        int goCount = 0;
        int jailCount = 0;

        for (const BoardTileConfig& tile : boardLayout) {
            const std::string type = toLower(tile.type);
            const bool knownType =
                type == "go" || type == "jail" || type == "go_to_jail" ||
                type == "free_parking" || type == "chance" ||
                type == "community_chest" || type == "festival" ||
                type == "pph_tax" || type == "pph" || type == "pbm_tax" ||
                type == "pbm" || type == "property";
            if (!knownType) {
                throw InvalidBoardConfigurationException(
                    "Tipe petak '" + tile.type + "' pada kode " + tile.code +
                    " tidak dikenali.");
            }

            if (type == "go") {
                goCount++;
            } else if (type == "jail") {
                jailCount++;
            } else if (type == "property") {
                const std::string refCode =
                    tile.propertyCode.empty() ? tile.code : tile.propertyCode;
                if (propertyConfigs.find(toUpper(refCode)) ==
                    propertyConfigs.end()) {
                    throw InvalidBoardConfigurationException(
                        "Properti " + toUpper(refCode) + " pada petak " +
                        tile.code + " tidak ditemukan di property.txt.");
                }
            }
        }

        if (goCount != 1) {
            throw InvalidBoardConfigurationException(
                "Harus ada tepat 1 petak GO, ditemukan " +
                std::to_string(goCount) + ".");
        }

        if (jailCount != 1) {
            throw InvalidBoardConfigurationException(
                "Harus ada tepat 1 petak Penjara/JAIL, ditemukan " +
                std::to_string(jailCount) + ".");
        }

        lastError.clear();
    } catch (const NimonspoliException& error) {
        setLastError(error.what());
        throw;
    } catch (const std::exception& error) {
        const InvalidBoardConfigurationException wrapped(error.what());
        setLastError(wrapped.what());
        throw wrapped;
    }
}

void Configuration::validatePropertyConfig(const PropertyConfig& config) const {
    config.validate();
}

void Configuration::setLastError(const std::string& message) {
    lastError = message;
}

std::string Configuration::toUpper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return text;
}

std::string Configuration::toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

char Configuration::firstNonWhitespaceChar(const std::string& line) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = line.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return '\0';
    }
    return line[start];
}
