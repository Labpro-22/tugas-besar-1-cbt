#pragma once

#include <string>
#include <vector>

class ConfigRecord {
public:
    virtual ~ConfigRecord() = default;

    virtual void loadFromLine(const std::string& line) = 0;
    virtual void validate() const = 0;
    virtual std::string toString() const = 0;

protected:
    static std::string trim(const std::string& text);
    static std::string toUpper(std::string text);
    static std::string toLower(std::string text);
    static std::string decodeName(std::string token);
    static std::vector<std::string> tokenize(const std::string& line);
    static int parseInt(const std::string& token, const std::string& fieldName);
    static std::string joinInts(const std::vector<int>& values);
    static std::string formatPair(int first, int second);
    static std::string formatTriple(int first, int second, int third);
};
