#include "views/InputHandler.hpp"

#include <sstream>

InputHandler::InputHandler(std::istream& in, std::ostream& out)
    : in(&in), out(&out) {}

std::string InputHandler::readLine() {
    std::string line;
    std::getline(*in, line);
    return line;
}

int InputHandler::readInt(const std::string& prompt) {
    while (true) {
        *out << prompt;

        std::string line;
        std::getline(*in, line);

        std::stringstream ss(line);
        int value;
        char extra;

        if ((ss >> value) && !(ss >> extra)) {
            return value;
        }

        *out << "Input tidak valid. Masukkan bilangan bulat.\n";
    }
}

bool InputHandler::readYesNo(const std::string& prompt) {
    while (true) {
        *out << prompt;

        std::string line;
        std::getline(*in, line);

        if (line == "y" || line == "Y" || line == "yes" || line == "YES") {
            return true;
        }

        if (line == "n" || line == "N" || line == "no" || line == "NO") {
            return false;
        }

        *out << "Input tidak valid. Masukkan y/n.\n";
    }
}

int InputHandler::readChoice(int min, int max, const std::string& prompt) {
    while (true) {
        int value = readInt(prompt);

        if (value >= min && value <= max) {
            return value;
        }

        *out << "Pilihan harus di antara " << min << " dan " << max << ".\n";
    }
}

std::string InputHandler::readToken(const std::string& prompt) {
    while (true) {
        *out << prompt;

        std::string line;
        std::getline(*in, line);

        std::stringstream ss(line);
        std::string token;
        char extra;

        if ((ss >> token) && !(ss >> extra)) {
            return token;
        }

        *out << "Input tidak valid. Masukkan satu token tanpa spasi.\n";
    }
}