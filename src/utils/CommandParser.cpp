#include "utils/CommandParser.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

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

bool hasBalancedQuotes(const std::string& text) {
    return std::count(text.begin(), text.end(), '"') % 2 == 0;
}

std::vector<std::string> tokenizeCommand(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;

    for (char ch : input) {
        if (ch == '"') {
            inQuotes = !inQuotes;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current += ch;
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

std::string uppercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return text;
}

}  // namespace

CommandParser::CommandParser() {
    expectedArgCount["CETAK_PAPAN"] = 0;
    expectedArgCount["LEMPAR_DADU"] = 0;
    expectedArgCount["ATUR_DADU"] = 2;
    expectedArgCount["CETAK_AKTA"] = 0;
    expectedArgCount["CETAK_PROPERTI"] = 0;
    expectedArgCount["BELI"] = 0;
    expectedArgCount["BAYAR_SEWA"] = 0;
    expectedArgCount["BAYAR_PAJAK"] = 0;
    expectedArgCount["GADAI"] = 0;
    expectedArgCount["TEBUS"] = 0;
    expectedArgCount["BANGUN"] = 0;
    expectedArgCount["LELANG"] = 0;
    expectedArgCount["FESTIVAL"] = 0;
    expectedArgCount["SIMPAN"] = 1;
    expectedArgCount["MUAT"] = 1;
    expectedArgCount["CETAK_LOG"] = -1;
    expectedArgCount["GUNAKAN_KEMAMPUAN"] = 0;
    expectedArgCount["DROP_KARTU_KEMAMPUAN"] = 0;
    expectedArgCount["PASS"] = 0;
    expectedArgCount["BID"] = 1;
    expectedArgCount["NEW_GAME"] = 0;
    expectedArgCount["LOAD_GAME"] = -1;
    expectedArgCount["EXIT"] = 0;
}

std::string CommandParser::normalize(const std::string& raw) const {
    std::string normalized = trim(raw);
    if (normalized.empty()) {
        return "";
    }

    std::string result;
    bool inQuotes = false;
    bool previousSpace = false;

    for (char ch : normalized) {
        if (ch == '"') {
            inQuotes = !inQuotes;
            result += ch;
            previousSpace = false;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) && !inQuotes) {
            if (!previousSpace) {
                result += ' ';
                previousSpace = true;
            }
            continue;
        }

        result += ch;
        previousSpace = false;
    }

    return result;
}

Command CommandParser::parse(const std::string& input) const {
    std::string normalized = normalize(input);
    if (normalized.empty() || !hasBalancedQuotes(normalized)) {
        return Command();
    }

    std::vector<std::string> tokens = tokenizeCommand(normalized);
    if (tokens.empty()) {
        return Command();
    }

    std::string name = uppercase(tokens.front());
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    return Command(name, args);
}

bool CommandParser::validate(const Command& cmd) const {
    if (cmd.name.empty()) {
        return false;
    }

    auto it = expectedArgCount.find(cmd.name);
    if (it == expectedArgCount.end()) {
        return false;
    }

    int expected = it->second;

    if (expected == -1) {
        return cmd.args.size() <= 1;
    }

    return static_cast<int>(cmd.args.size()) == expected;
}
