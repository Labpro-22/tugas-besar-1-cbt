#include "core/Board-Tiles/FestivalTile.hpp"

#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "views/InputHandler.hpp"

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

FestivalTile::FestivalTile(const std::string &code, const std::string &name, int pos)
    : Tile(code, name, pos, "festival") {}

void FestivalTile::onLanded(Player &player, GameManager &game) {
    static std::unordered_map<PropertyTile *, int> festivalMultiplierByTile;

    Board &board = game.getBoard();
    std::vector<PropertyTile *> ownedTiles;
    ownedTiles.reserve(static_cast<std::size_t>(board.getTileCount()));

    for (int i = 0; i < board.getTileCount(); ++i) {
        Tile &tile = board.getTile(i);
        if (tile.getType() != "property") {
            continue;
        }

        PropertyTile &propertyTile = static_cast<PropertyTile &>(tile);
        Property &property = propertyTile.getProperty();
        if (property.getOwner() == &player) {
            ownedTiles.push_back(&propertyTile);
        }
    }

    if (ownedTiles.empty()) {
        std::cout << "Kamu mendarat di petak Festival!\n";
        std::cout << "Kamu belum memiliki properti untuk diberi efek festival.\n";
        return;
    }

    std::cout << "Kamu mendarat di petak Festival!\n\n";
    std::cout << "Daftar properti milikmu:\n";
    for (PropertyTile *tile : ownedTiles) {
        if (tile == nullptr) {
            continue;
        }

        std::cout << "- " << tile->getProperty().getCode() << " ("
                  << tile->getProperty().getName() << ")\n";
    }

    PropertyTile *selectedTile = nullptr;
    InputHandler input;
    while (selectedTile == nullptr) {
        std::string propertyCode = input.readPromptLine(
            "Masukkan kode properti: ", "Festival");

        for (PropertyTile *tile : ownedTiles) {
            if (tile != nullptr && tile->getProperty().getCode() == propertyCode) {
                selectedTile = tile;
                break;
            }
        }

        if (selectedTile == nullptr) {
            std::cout << "-> Kode properti tidak valid!\n";
        }
    }

    int currentMultiplier = 1;
    const auto multiplierIt = festivalMultiplierByTile.find(selectedTile);
    if (multiplierIt != festivalMultiplierByTile.end()) {
        currentMultiplier = multiplierIt->second;
    }

    int nextMultiplier = currentMultiplier;
    if (nextMultiplier < 8) {
        nextMultiplier *= 2;
    }

    if (currentMultiplier == 1) {
        std::cout << "\nEfek festival aktif!\n";
    } else if (nextMultiplier == currentMultiplier) {
        std::cout << "\nEfek sudah maksimum (harga sewa sudah digandakan tiga kali)\n";
    } else {
        std::cout << "\nEfek diperkuat!\n";
    }

    const int baseRent = selectedTile->getProperty().getPropertyDetail();
    festivalMultiplierByTile[selectedTile] = nextMultiplier;
    selectedTile->applyFestivalEffect(nextMultiplier, 3);

    if (currentMultiplier > 1) {
        std::cout << "Sewa sebelumnya: M" << (baseRent * currentMultiplier) << "\n";
    } else {
        std::cout << "Sewa awal: M" << baseRent << "\n";
    }
    std::cout << "Sewa sekarang: M" << (baseRent * nextMultiplier) << "\n";
    std::cout << "Durasi: 3 giliran\n";
}
