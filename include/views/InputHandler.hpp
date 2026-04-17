#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <iostream>
#include <string>

class InputHandler {
private:
    std::istream* in;
    std::ostream* out;

public:
    InputHandler(std::istream& in = std::cin, std::ostream& out = std::cout);

    std::string readLine();
    int readInt(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    int readChoice(int min, int max, const std::string& prompt);
    std::string readToken(const std::string& prompt);
};

#endif