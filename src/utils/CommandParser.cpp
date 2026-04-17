#include "utils/CommandParser.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

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
}

std::string CommandParser::normalize(const std::string& raw) const {
    std::stringstream ss(raw);
    std::string token;
    std::string result;

    while (ss >> token) {
        if (!result.empty()) {
            result += ' ';
        }
        result += token;
    }

    return result;
}

Command CommandParser::parse(const std::string& input) const {
    std::string normalized = normalize(input);
    std::stringstream ss(normalized);

    std::string name;
    ss >> name;

    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg) {
        args.push_back(arg);
    }

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