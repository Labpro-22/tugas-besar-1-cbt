#ifndef APP_GAME_SESSION_UTIL_HPP
#define APP_GAME_SESSION_UTIL_HPP

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "../core/Board-Tiles/PropertyTile.hpp"
#include "../core/Board-Tiles/Tile.hpp"
#include "../models/GameManager/Player.hpp"
#include "../models/Property/Street.hpp"

class GameSessionUtil {
public:
    static std::string uppercase(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
                       [](unsigned char ch) {
                           return static_cast<char>(std::toupper(ch));
                       });
        return text;
    }

    static std::string lowercase(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
                       [](unsigned char ch) {
                           return static_cast<char>(std::tolower(ch));
                       });
        return text;
    }

    static std::string trim(const std::string& text) {
        const std::string whitespace = " \t\r\n";
        const std::size_t start = text.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            return "";
        }

        const std::size_t end = text.find_last_not_of(whitespace);
        return text.substr(start, end - start + 1);
    }

    static std::vector<std::string> tokenize(const std::string& line) {
        std::stringstream ss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (ss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    static std::string playerStatusLabel(const Player& player) {
        switch (player.getStatus()) {
            case ACTIVE:
                return "ACTIVE";
            case BANKRUPT:
                return "BANKRUPT";
            case JAILED:
                return "JAILED";
        }

        return "UNKNOWN";
    }

    static std::string colorGroupKey(ColorGroup color) {
        switch (color) {
            case ColorGroup::COKLAT:
                return "CK";
            case ColorGroup::BIRU_MUDA:
                return "BM";
            case ColorGroup::MERAH_MUDA:
                return "PK";
            case ColorGroup::ORANGE:
                return "OR";
            case ColorGroup::MERAH:
                return "MR";
            case ColorGroup::KUNING:
                return "KN";
            case ColorGroup::HIJAU:
                return "HJ";
            case ColorGroup::BIRU_TUA:
                return "BT";
            case ColorGroup::ABU_ABU:
                return "AB";
        }

        return "DF";
    }

    static std::string colorGroupLabel(ColorGroup color) {
        switch (color) {
            case ColorGroup::COKLAT:
                return "COKLAT";
            case ColorGroup::BIRU_MUDA:
                return "BIRU MUDA";
            case ColorGroup::MERAH_MUDA:
                return "PINK";
            case ColorGroup::ORANGE:
                return "ORANGE";
            case ColorGroup::MERAH:
                return "MERAH";
            case ColorGroup::KUNING:
                return "KUNING";
            case ColorGroup::HIJAU:
                return "HIJAU";
            case ColorGroup::BIRU_TUA:
                return "BIRU TUA";
            case ColorGroup::ABU_ABU:
                return "ABU-ABU";
        }

        return "DEFAULT";
    }

    static std::string resolveTileColorKey(const Tile& tile) {
        if (tile.getType() == "property") {
            const PropertyTile& propTile = static_cast<const PropertyTile&>(tile);
            const Property& property = propTile.getProperty();
            if (property.getType() == "Street") {
                const Street& street = static_cast<const Street&>(property);
                return colorGroupKey(street.getColorGroup());
            }
            if (property.getType() == "Utility") {
                return "AB";
            }
            if (property.getType() == "Railroad") {
                return "DF";
            }
        }

        if (tile.getType() == "festival") {
            return "FV";
        }
        return "DF";
    }
};

#endif
