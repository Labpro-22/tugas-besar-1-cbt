#include "views/PropertyView.hpp"

#include <iostream>

PropertyView::PropertyView(int cardWidth) : cardWidth(cardWidth) {}

void PropertyView::showDeed(Property* property) const {
    if (property == nullptr) {
        std::cout << "Properti tidak valid.\n";
        return;
    }

    std::cout << "+" << std::string(cardWidth, '=') << "+\n";
    std::cout << "| AKTA KEPEMILIKAN\n";
    std::cout << "| " << property->getName() << " (" << property->getCode() << ")\n";
    std::cout << "+" << std::string(cardWidth, '=') << "+\n";
    std::cout << "| Harga Beli   : M" << property->getBuyPrice() << "\n";
    std::cout << "| Nilai Gadai  : M" << property->getMortgageValue() << "\n";
    std::cout << "+" << std::string(cardWidth, '=') << "+\n";
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
                      << " | Harga Beli: M" << property->getBuyPrice()
                      << " | Gadai: M" << property->getMortgageValue()
                      << "\n";
        }
    }
}

bool PropertyView::showBuyPrompt(Property* property, int playerCash) const {
    if (property == nullptr) {
        std::cout << "Properti tidak valid.\n";
        return false;
    }

    std::cout << "Kamu mendarat di " << property->getName()
              << " (" << property->getCode() << ")!\n";
    std::cout << "Harga beli : M" << property->getBuyPrice() << "\n";
    std::cout << "Uang kamu  : M" << playerCash << "\n";
    std::cout << "Apakah kamu ingin membeli properti ini? (y/n): ";

    std::string answer;
    std::cin >> answer;

    return (answer == "y" || answer == "Y");
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
                      << " | Harga Tebus: M" << mortgaged[i]->getBuyPrice()
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