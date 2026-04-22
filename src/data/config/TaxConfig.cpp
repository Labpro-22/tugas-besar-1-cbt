#include "data/config/TaxConfig.hpp"

#include "exception/NimonspoliExceptions.hpp"

TaxConfig::TaxConfig() = default;

TaxConfig::TaxConfig(const std::string& line) {
    loadFromLine(line);
}

void TaxConfig::loadFromLine(const std::string& line) {
    try {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 3) {
            throw InvalidConfigurationFormatException(
                "tax config", "Baris pajak harus berisi PPH flat, persentase PPH, dan PBM flat.");
        }

        pphFlat = parseInt(tokens[0], "pphFlat");
        pphPercentage = parseInt(tokens[1], "pphPercentage");
        pbmFlat = parseInt(tokens[2], "pbmFlat");

        validate();
    } catch (const NimonspoliException&) {
        throw;
    } catch (const std::exception& error) {
        throw InvalidConfigurationFormatException("tax config", error.what());
    }
}

void TaxConfig::validate() const {
    if (pphFlat < 0 || pphPercentage < 0 || pbmFlat < 0) {
        throw InvalidConfigurationFormatException(
            "tax config", "Nilai pajak tidak boleh negatif.");
    }
}

std::string TaxConfig::toString() const {
    return formatTriple(pphFlat, pphPercentage, pbmFlat);
}
