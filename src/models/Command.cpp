#include "models/Command.hpp"

Command::Command() : name(""), args() {}

Command::Command(const std::string& name, const std::vector<std::string>& args)
    : name(name), args(args) {}