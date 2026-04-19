#include "PropertyView.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "InputHandler.hpp"
#include "../models/GameManager/Player.hpp"
#include "../models/Property/Property.hpp"
#include "../models/Property/Railroad.hpp"
#include "../models/Property/Street.hpp"
#include "../models/Property/Utility.hpp"

namespace {

std::string normalizeAnswer(std::string answer) {
    std::transform(answer.begin(), answer.end(), answer.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return answer;
}

int resolveCardWidth(const Property* property, int preferredWidth) {
    if (property == nullptr) {
        return preferredWidth;
    }

    const int minimumWidth = static_cast<int>(property->getName().size() + property->getCode().size()) + 8;
    return std::max(preferredWidth, minimumWidth);
}

std::string colorGroupLabel(ColorGroup color) {
    switch (color) {
        case ColorGroup::COKLAT:
            return "COKLAT";
        case ColorGroup::BIRU_MUDA:
            return "BIRU MUDA";
        case ColorGroup::MERAH_MUDA:
            return "PINK";
        case ColorGroup::ORANGE:
            return "ORANGE";
        case ColorGroup::MERAH:
            return "MERAH";
        case ColorGroup::KUNING:
            return "KUNING";
        case ColorGroup::HIJAU:
            return "HIJAU";
        case ColorGroup::BIRU_TUA:
            return "BIRU TUA";
        case ColorGroup::ABU_ABU:
            return "ABU-ABU";
    }

    return "TIDAK DIKETAHUI";
}

std::string buildingLabel(const Property* property) {
    const Street* street = dynamic_cast<const Street*>(property);
    if (street == nullptr) {
        return "-";
    }

    const int buildingCount = street->getBuildingCount();
    if (buildingCount <= 0) {
        return "Tanpa bangunan";
    }
    if (buildingCount >= static_cast<int>(BuildingLevel::HOTEL)) {
        return "Hotel";
    }

    std::ostringstream oss;
    oss << buildingCount << " rumah";
    return oss.str();
}

}  // namespace

PropertyView::PropertyView(int cardWidth) : cardWidth(cardWidth) {}

void PropertyView::showDeed(Property* property) const {
    if (property == nullptr) {
        std::cout << "Properti tidak valid.\n";
        return;
    }

    const int width = resolveCardWidth(property, cardWidth);

    std::cout << "+" << std::string(width, '=') << "+\n";
    std::cout << "|        AKTA KEPEMILIKAN\n";

    if (const Street* street = dynamic_cast<const Street*>(property)) {
        std::cout << "|    [" << colorGroupLabel(street->getColorGroup()) << "] "
                  << property->getName() << " (" << property->getCode() << ")\n";
    } else {
        std::cout << "|    " << property->getName() << " (" << property->getCode() << ")\n";
    }

    std::cout << "+" << std::string(width, '=') << "+\n";
    std::cout << "| Harga Beli        : M" << property->getBuyPrice() << "\n";
    std::cout << "| Nilai Gadai       : M" << property->getMortgageValue() << "\n";
    std::cout << "+--------------------------------+\n";

    if (const Street* street = dynamic_cast<const Street*>(property)) {
        const std::vector<int>& rents = street->getRentLevels();
        if (!rents.empty()) {
            std::cout << "| Sewa (unimproved) : M" << rents[0] << "\n";
        }
        if (rents.size() > 1) {
            std::cout << "| Sewa (1 rumah)    : M" << rents[1] << "\n";
        }
        if (rents.size() > 2) {
            std::cout << "| Sewa (2 rumah)    : M" << rents[2] << "\n";
        }
        if (rents.size() > 3) {
            std::cout << "| Sewa (3 rumah)    : M" << rents[3] << "\n";
        }
        if (rents.size() > 4) {
            std::cout << "| Sewa (4 rumah)    : M" << rents[4] << "\n";
        }
        if (rents.size() > 5) {
            std::cout << "| Sewa (hotel)      : M" << rents[5] << "\n";
        }
        std::cout << "+--------------------------------+\n";
        std::cout << "| Harga Rumah       : M" << street->getHouseCost() << "\n";
        std::cout << "| Harga Hotel       : M" << street->getHotelCost() << "\n";
    } else if (const Railroad* railroad = dynamic_cast<const Railroad*>(property)) {
        for (const auto& [ownedCount, rent] : railroad->getRentTable()) {
            std::cout << "| Sewa (" << ownedCount << " stasiun)    : M" << rent << "\n";
        }
    } else if (const Utility* utility = dynamic_cast<const Utility*>(property)) {
        for (const auto& [ownedCount, multiplier] : utility->getMultiplierTable()) {
            std::cout << "| Multiplier (" << ownedCount << " util) : x" << multiplier << "\n";
        }
    }

    std::cout << "+" << std::string(width, '=') << "+\n";
    std::cout << "| Status : " << property->getStatusString();
    if (property->getOwner() != nullptr) {
        std::cout << " (" << property->getOwner()->getUsername() << ")";
    }
    std::cout << "\n";

    if (property->getFMult() > 1) {
        std::cout << "| Festival aktif x" << property->getFMult()
                  << " | sisa " << property->getFDur() << " turn\n";
    }

    std::cout << "+" << std::string(width, '=') << "+\n";
}

void PropertyView::showPlayerProperties(Player* player) const {
    if (player == nullptr) {
        std::cout << "Data pemain tidak valid.\n";
        return;
    }

    std::cout << "=== Properti Milik: " << player->getUsername() << " ===\n";

    const std::vector<Property*>& properties = player->getProperties();
    if (properties.empty()) {
        std::cout << "Kamu belum memiliki properti apapun.\n";
        return;
    }

    for (Property* property : properties) {
        if (property != nullptr) {
            std::cout << "- " << property->getName()
                      << " (" << property->getCode() << ")"
                      << " | " << buildingLabel(property)
                      << " | M" << property->getBuyPrice()
                      << " | " << property->getStatusString();

            if (property->getFMult() > 1) {
                std::cout << " | Festival x" << property->getFMult()
                          << " (" << property->getFDur() << " turn)";
            }

            std::cout << " | Gadai: M" << property->getMortgageValue()
                      << "\n";
        }
    }
}

bool PropertyView::showBuyPrompt(Property* property, int playerCash) const {
    InputHandler input;
    return showBuyPrompt(property, playerCash, input);
}

bool PropertyView::showBuyPrompt(Property* property, int playerCash, InputHandler& input) const {
    if (property == nullptr) {
        std::cout << "Properti tidak valid.\n";
        return false;
    }

    while (true) {
        std::cout << "Kamu mendarat di " << property->getName()
                  << " (" << property->getCode() << ")!\n";
        std::cout << "Harga beli : M" << property->getBuyPrice() << "\n";
        std::cout << "Uang kamu  : M" << playerCash << "\n";
        std::string answer = normalizeAnswer(
            input.readPromptLine("Apakah kamu ingin membeli properti ini? (y/n): ",
                                 "Konfirmasi Pembelian"));

        if (answer == "y" || answer == "yes") {
            return true;
        }

        if (answer == "n" || answer == "no") {
            return false;
        }

        std::cout << "Input tidak valid. Masukkan y/n.\n";
    }
}

void PropertyView::showMortgageOptions(const std::vector<Property*>& properties) const {
    std::cout << "=== Properti yang Dapat Digadaikan ===\n";

    if (properties.empty()) {
        std::cout << "Tidak ada properti yang dapat digadaikan saat ini.\n";
        return;
    }

    for (std::size_t i = 0; i < properties.size(); ++i) {
        if (properties[i] != nullptr) {
            std::cout << (i + 1) << ". "
                      << properties[i]->getName()
                      << " (" << properties[i]->getCode() << ")"
                      << " | Nilai Gadai: M" << properties[i]->getMortgageValue()
                      << "\n";
        }
    }
}

void PropertyView::showRedeemOptions(const std::vector<Property*>& mortgaged) const {
    std::cout << "=== Properti yang Sedang Digadaikan ===\n";

    if (mortgaged.empty()) {
        std::cout << "Tidak ada properti yang sedang digadaikan.\n";
        return;
    }

    for (std::size_t i = 0; i < mortgaged.size(); ++i) {
        if (mortgaged[i] != nullptr) {
            std::cout << (i + 1) << ". "
                      << mortgaged[i]->getName()
                      << " (" << mortgaged[i]->getCode() << ")"
                      << " | Harga Tebus: M"
                      << (mortgaged[i]->getMortgageValue() * 2)
                      << "\n";
        }
    }
}

void PropertyView::showBuildOptions(const std::map<std::string, std::vector<Property*>>& buildableGroups) const {
    std::cout << "=== Color Group yang Memenuhi Syarat ===\n";

    if (buildableGroups.empty()) {
        std::cout << "Tidak ada color group yang memenuhi syarat untuk dibangun.\n";
        return;
    }

    int idx = 1;
    for (const auto& entry : buildableGroups) {
        std::cout << idx++ << ". [" << entry.first << "]\n";

        for (Property* property : entry.second) {
            if (property != nullptr) {
                std::cout << "   - " << property->getName()
                          << " (" << property->getCode() << ")\n";
            }
        }
    }
}
