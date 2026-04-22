#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <map>
#include <string>

#include "../models/Command.hpp"

class CommandParser {
private:
    std::map<std::string, int> expectedArgCount;

public:
    CommandParser();

    Command parse(const std::string& input) const;
    bool validate(const Command& cmd) const;
    std::string normalize(const std::string& raw) const;
};

#endif
