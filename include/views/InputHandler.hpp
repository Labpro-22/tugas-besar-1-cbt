#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <functional>
#include <iostream>
#include <string>
#include <mutex>
#include <utility>
#include <string>

enum class InputPromptKind {
    Line,
    Token,
    Integer,
    Choice,
    YesNo,
};

class InputPromptRequest {
public:
    InputPromptKind kind = InputPromptKind::Line;
    std::string title;
    std::string prompt;
    std::string initialValue;
    int minValue = 0;
    int maxValue = 0;
};

class InputPromptResponse {
public:
    bool accepted = false;
    std::string value;
};

class InputHandler {
private:
    std::istream* in;
    std::ostream* out;

    static std::mutex providerMutex;
    static std::function<InputPromptResponse(const InputPromptRequest&)> promptProvider;

    bool tryPrompt(const InputPromptRequest& request,
                   InputPromptResponse& response) const;

public:
    InputHandler(std::istream& in = std::cin, std::ostream& out = std::cout);

    static void setPromptProvider(
        std::function<InputPromptResponse(const InputPromptRequest&)> provider);
    static void clearPromptProvider();

    bool isStreamGood() const;
    std::string readLine();
    std::string readPromptLine(const std::string& prompt,
                               const std::string& title = "Input");
    int readInt(const std::string& prompt);
    bool readYesNo(const std::string& prompt);
    int readChoice(int min, int max, const std::string& prompt);
    std::string readToken(const std::string& prompt);
};

#endif
