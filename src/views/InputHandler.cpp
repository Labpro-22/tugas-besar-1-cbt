#include "views/InputHandler.hpp"

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

std::mutex InputHandler::providerMutex;
std::function<InputPromptResponse(const InputPromptRequest&)>
    InputHandler::promptProvider;

InputHandler::InputHandler(std::istream& in, std::ostream& out)
    : in(&in), out(&out) {}

void InputHandler::setPromptProvider(
    std::function<InputPromptResponse(const InputPromptRequest&)> provider) {
    std::lock_guard<std::mutex> lock(providerMutex);
    promptProvider = std::move(provider);
}

void InputHandler::clearPromptProvider() {
    std::lock_guard<std::mutex> lock(providerMutex);
    promptProvider = nullptr;
}

bool InputHandler::tryPrompt(const InputPromptRequest& request,
                             InputPromptResponse& response) const {
    std::function<InputPromptResponse(const InputPromptRequest&)> providerCopy;
    {
        std::lock_guard<std::mutex> lock(providerMutex);
        providerCopy = promptProvider;
    }

    if (!providerCopy) {
        return false;
    }

    response = providerCopy(request);
    return true;
}

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

std::string InputHandler::readPromptLine(const std::string& prompt,
                                         const std::string& title) {
    while (true) {
        InputPromptResponse response;
        if (tryPrompt({InputPromptKind::Line, title, prompt, "", 0, 0},
                      response)) {
            if (response.accepted) {
                return trim(response.value);
            }

            if (out != nullptr) {
                *out << "Input dibatalkan. Silakan coba lagi.\n";
            }
            continue;
        }

        if (out != nullptr) {
            *out << prompt;
        }

        std::string line = readLine();
        if (line.empty() && !isStreamGood()) {
            throw std::runtime_error("Input stream closed.");
        }
        return line;
    }
}

int InputHandler::readInt(const std::string& prompt) {
    while (true) {
        std::string line;
        InputPromptResponse response;
        if (tryPrompt({InputPromptKind::Integer, "Input Angka", prompt, "", 0, 0},
                      response)) {
            if (!response.accepted) {
                if (out != nullptr) {
                    *out << "Input dibatalkan. Silakan coba lagi.\n";
                }
                continue;
            }
            line = trim(response.value);
        } else {
            if (out != nullptr) {
                *out << prompt;
            }
            line = readLine();
            if (line.empty() && !isStreamGood()) {
                throw std::runtime_error("Input stream closed.");
            }
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
        std::string line;
        InputPromptResponse response;
        if (tryPrompt({InputPromptKind::YesNo, "Konfirmasi", prompt, "", 0, 0},
                      response)) {
            if (!response.accepted) {
                if (out != nullptr) {
                    *out << "Input dibatalkan. Silakan coba lagi.\n";
                }
                continue;
            }
            line = toLower(trim(response.value));
        } else {
            if (out != nullptr) {
                *out << prompt;
            }

            line = toLower(readLine());
            if (line.empty() && !isStreamGood()) {
                throw std::runtime_error("Input stream closed.");
            }
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
        std::string line;
        InputPromptResponse response;
        if (tryPrompt({InputPromptKind::Choice, "Pilih Opsi", prompt, "", min, max},
                      response)) {
            if (!response.accepted) {
                if (out != nullptr) {
                    *out << "Input dibatalkan. Silakan coba lagi.\n";
                }
                continue;
            }
            line = trim(response.value);
        } else {
            line = std::to_string(readInt(prompt));
        }

        std::stringstream ss(line);
        int value;
        char extra;
        if (!((ss >> value) && !(ss >> extra))) {
            if (out != nullptr) {
                *out << "Input tidak valid. Masukkan bilangan bulat.\n";
            }
            continue;
        }

        if (value >= min && value <= max) {
            return value;
        }

        if (out != nullptr) {
            *out << "Pilihan harus di antara " << min << " dan " << max << ".\n";
        }
    }
}

std::string InputHandler::readToken(const std::string& prompt) {
    while (true) {
        std::string line;
        InputPromptResponse response;
        if (tryPrompt({InputPromptKind::Token, "Input Teks", prompt, "", 0, 0},
                      response)) {
            if (!response.accepted) {
                if (out != nullptr) {
                    *out << "Input dibatalkan. Silakan coba lagi.\n";
                }
                continue;
            }
            line = trim(response.value);
        } else {
            if (out != nullptr) {
                *out << prompt;
            }

            line = readLine();
            if (line.empty() && !isStreamGood()) {
                throw std::runtime_error("Input stream closed.");
            }
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
