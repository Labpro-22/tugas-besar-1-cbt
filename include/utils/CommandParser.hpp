#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <map>
#include <string>
#include <vector>

#include "../models/Command.hpp"

class CommandParser {
private:
    std::map<std::string, int> expectedArgCount;

    static std::string trim(const std::string& text);
    static bool hasBalancedQuotes(const std::string& text);
    static std::vector<std::string> tokenizeCommand(const std::string& input);
    static std::string uppercase(std::string text);

public:
    CommandParser();

    Command parse(const std::string& input) const;
    bool validate(const Command& cmd) const;
    std::string normalize(const std::string& raw) const;
};

#endif
