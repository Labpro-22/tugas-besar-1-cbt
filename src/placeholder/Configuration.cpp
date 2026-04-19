#include "Configuration.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::string trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }

    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

std::string toUpper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return text;
}

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string decodeName(std::string token) {
    std::replace(token.begin(), token.end(), '_', ' ');
    return token;
}

bool isDataLine(const std::string& line) {
    const std::string trimmed = trim(line);
    if (trimmed.empty()) {
        return false;
    }
    if (trimmed[0] == '#') {
        return false;
    }
    return true;
}

std::vector<std::string> tokenize(const std::string& line) {
    std::stringstream ss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool parsePairFile(const std::filesystem::path& path, std::map<int, int>& target) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    target.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (!isDataLine(line)) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2 || !std::isdigit(static_cast<unsigned char>(tokens[0][0]))) {
            continue;
        }

        target[std::stoi(tokens[0])] = std::stoi(tokens[1]);
    }

    return !target.empty();
}

}  // namespace

Configuration::Configuration() : configDir("config"), loaded(false) {}

Configuration::Configuration(const std::string& dir)
    : configDir(dir), loaded(false) {}

bool Configuration::loadAllConfigs() {
    reset();

    if (!loadProperties()) {
        return false;
    }
    if (!loadRailroadConfig()) {
        return false;
    }
    if (!loadUtilityConfig()) {
        return false;
    }
    if (!loadTaxConfig()) {
        return false;
    }
    if (!loadSpecialConfig()) {
        return false;
    }
    if (!loadMiscConfig()) {
        return false;
    }
    if (!loadBoardLayout()) {
        return false;
    }
    if (!validateBoardLayout()) {
        return false;
    }

    loaded = true;
    return true;
}

bool Configuration::isConfigLoaded() const { return loaded; }

const std::string& Configuration::getConfigDir() const { return configDir; }

const std::vector<BoardTileConfig>& Configuration::getBoardLayout() const {
    return boardLayout;
}

const std::map<int, int>& Configuration::getRailroadRentTable() const {
    return railroadRentTable;
}

const std::map<int, int>& Configuration::getUtilityMultiplierTable() const {
    return utilityMultiplierTable;
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

const TaxConfig& Configuration::getTaxConfig() const { return taxConfig; }

const SpecialConfig& Configuration::getSpecialConfig() const { return specialConfig; }

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
}

bool Configuration::loadProperties() {
    std::ifstream file(std::filesystem::path(configDir) / "property.txt");
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isDataLine(line)) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 7 || !std::isdigit(static_cast<unsigned char>(tokens[0][0]))) {
            continue;
        }

        PropertyConfig property;
        property.id = std::stoi(tokens[0]);
        property.code = toUpper(tokens[1]);
        property.name = decodeName(tokens[2]);
        property.propertyType = toUpper(tokens[3]);
        property.colorGroup = toUpper(tokens[4]);
        property.buyPrice = std::stoi(tokens[5]);
        property.mortgageValue = std::stoi(tokens[6]);

        if (property.propertyType == "STREET") {
            if (tokens.size() < 15) {
                return false;
            }

            property.houseUpgradeCost = std::stoi(tokens[7]);
            property.hotelUpgradeCost = std::stoi(tokens[8]);
            for (std::size_t i = 9; i < tokens.size(); ++i) {
                property.rentLevels.push_back(std::stoi(tokens[i]));
            }
        }

        propertyConfigs[property.code] = property;
    }

    return !propertyConfigs.empty();
}

bool Configuration::loadRailroadConfig() {
    return parsePairFile(std::filesystem::path(configDir) / "railroad.txt",
                         railroadRentTable);
}

bool Configuration::loadUtilityConfig() {
    return parsePairFile(std::filesystem::path(configDir) / "utility.txt",
                         utilityMultiplierTable);
}

bool Configuration::loadTaxConfig() {
    std::ifstream file(std::filesystem::path(configDir) / "tax.txt");
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isDataLine(line)) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 3 || !std::isdigit(static_cast<unsigned char>(tokens[0][0]))) {
            continue;
        }

        taxConfig.pphFlat = std::stoi(tokens[0]);
        taxConfig.pphPercentage = std::stoi(tokens[1]);
        taxConfig.pbmFlat = std::stoi(tokens[2]);
        return true;
    }

    return false;
}

bool Configuration::loadSpecialConfig() {
    std::ifstream file(std::filesystem::path(configDir) / "special.txt");
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isDataLine(line)) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2 || !std::isdigit(static_cast<unsigned char>(tokens[0][0]))) {
            continue;
        }

        specialConfig.goSalary = std::stoi(tokens[0]);
        specialConfig.jailFine = std::stoi(tokens[1]);
        return true;
    }

    return false;
}

bool Configuration::loadMiscConfig() {
    std::ifstream file(std::filesystem::path(configDir) / "misc.txt");
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isDataLine(line)) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 2 || !std::isdigit(static_cast<unsigned char>(tokens[0][0]))) {
            continue;
        }

        miscConfig.maxTurn = std::stoi(tokens[0]);
        miscConfig.startingCash = std::stoi(tokens[1]);
        return true;
    }

    return false;
}

bool Configuration::loadBoardLayout() {
    const std::filesystem::path preferred = std::filesystem::path(configDir) / "board.txt";
    const std::filesystem::path fallback = std::filesystem::path(configDir) / "board_layout.txt";

    std::ifstream file(preferred);
    if (!file.is_open()) {
        file.open(fallback);
    }
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isDataLine(line)) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 3) {
            continue;
        }

        BoardTileConfig tile;
        tile.code = toUpper(tokens[0]);
        tile.name = decodeName(tokens[1]);
        tile.type = toLower(tokens[2]);
        if (tokens.size() > 3) {
            tile.propertyCode = toUpper(tokens[3]);
        }

        boardLayout.push_back(tile);
    }

    return !boardLayout.empty();
}

bool Configuration::validateBoardLayout() const {
    if (boardLayout.size() < 20 || boardLayout.size() > 60) {
        return false;
    }

    int goCount = 0;
    int jailCount = 0;

    for (const BoardTileConfig& tile : boardLayout) {
        const std::string type = toLower(tile.type);
        if (type == "go") {
            goCount++;
        } else if (type == "jail") {
            jailCount++;
        } else if (type == "property") {
            const std::string refCode =
                tile.propertyCode.empty() ? tile.code : tile.propertyCode;
            if (propertyConfigs.find(toUpper(refCode)) == propertyConfigs.end()) {
                return false;
            }
        }
    }

    return goCount == 1 && jailCount == 1;
}
