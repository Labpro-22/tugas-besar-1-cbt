#include "InputHandler.hpp"

#include <algorithm>
#include <cctype>
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

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return text;
}

}  // namespace

InputHandler::InputHandler(std::istream& in, std::ostream& out)
    : in(&in), out(&out) {}

bool InputHandler::isStreamGood() const {
    return in != nullptr && static_cast<bool>(*in);
}

std::string InputHandler::readLine() {
    std::string line;
    if (!std::getline(*in, line)) {
        return "";
    }
    return trim(line);
}

int InputHandler::readInt(const std::string& prompt) {
    while (true) {
        *out << prompt;

        std::string line = readLine();
        if (line.empty() && !isStreamGood()) {
            throw std::runtime_error("Input stream closed.");
        }

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

        std::string line = toLower(readLine());
        if (line.empty() && !isStreamGood()) {
            throw std::runtime_error("Input stream closed.");
        }

        if (line == "y" || line == "yes" || line == "ya") {
            return true;
        }

        if (line == "n" || line == "no" || line == "tidak") {
            return false;
        }

        *out << "Input tidak valid. Masukkan y/n.\n";
    }
}

int InputHandler::readChoice(int min, int max, const std::string& prompt) {
    if (min > max) {
        std::swap(min, max);
    }

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

        std::string line = readLine();
        if (line.empty() && !isStreamGood()) {
            throw std::runtime_error("Input stream closed.");
        }

        std::stringstream ss(line);
        std::string token;
        char extra;

        if ((ss >> token) && !(ss >> extra)) {
            return token;
        }

        *out << "Input tidak valid. Masukkan satu token tanpa spasi.\n";
    }
}
