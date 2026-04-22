#include "data/config/ConfigRecord.hpp"

#include "exception/NimonspoliExceptions.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

std::string ConfigRecord::trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }

    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

std::string ConfigRecord::toUpper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return text;
}

std::string ConfigRecord::toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string ConfigRecord::decodeName(std::string token) {
    std::replace(token.begin(), token.end(), '_', ' ');
    return token;
}

std::vector<std::string> ConfigRecord::tokenize(const std::string& line) {
    std::stringstream ss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int ConfigRecord::parseInt(const std::string& token,
                           const std::string& fieldName) {
    try {
        std::size_t parsed = 0;
        const int value = std::stoi(token, &parsed);
        if (parsed != token.size()) {
            throw InvalidConfigurationFormatException(
                "config", "Nilai " + fieldName + " harus berupa bilangan bulat.");
        }
        return value;
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception&) {
        throw InvalidConfigurationFormatException(
            "config", "Nilai " + fieldName + " harus berupa bilangan bulat.");
    }
}

std::string ConfigRecord::joinInts(const std::vector<int>& values) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            oss << ' ';
        }
        oss << values[i];
    }
    return oss.str();
}

std::string ConfigRecord::formatPair(int first, int second) {
    std::ostringstream oss;
    oss << first << ' ' << second;
    return oss.str();
}

std::string ConfigRecord::formatTriple(int first, int second, int third) {
    std::ostringstream oss;
    oss << first << ' ' << second << ' ' << third;
    return oss.str();
}
