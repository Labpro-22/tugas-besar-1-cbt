#include "core/Board-Tiles/FestivalTile.hpp"

#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "views/InputHandler.hpp"
#include "exception/NimonspoliExceptions.hpp"

#include <iostream>
#include <string>
#include <vector>

FestivalTile::FestivalTile(const std::string &code, const std::string &name, int pos)
    : Tile(code, name, pos, "festival") {}

void FestivalTile::onLanded(Player &player, GameManager &game) {
    try {
        Board &board = game.getBoard();
        if (board.getTileCount() <= 0) {
            throw InvalidBoardConfigurationException("Board tidak memiliki petak.");
        }

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

        int currentMultiplier = selectedTile->getProperty().getFMult();
        if (currentMultiplier < 1) currentMultiplier = 1;
        if (!(currentMultiplier == 1 || currentMultiplier == 2 ||
              currentMultiplier == 4 || currentMultiplier == 8)) {
            throw PropertyException("Nilai festival multiplier properti tidak valid.",
                                    "PROPERTY_FESTIVAL_ERROR");
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
        if (baseRent < 0) {
            throw PropertyException("Nilai sewa properti tidak valid.",
                                    "PROPERTY_RENT_ERROR");
        }

        selectedTile->applyFestivalEffect(nextMultiplier, 3);
        game.executeFestival(player, selectedTile->getProperty().getCode());

        if (currentMultiplier > 1) {
            std::cout << "Sewa sebelumnya: M" << (baseRent * currentMultiplier) << "\n";
        } else {
            std::cout << "Sewa awal: M" << baseRent << "\n";
        }
        std::cout << "Sewa sekarang: M" << (baseRent * nextMultiplier) << "\n";
        std::cout << "Durasi: 3 giliran\n";
    } catch (const NimonspoliException &) {
        throw;
    } catch (const std::exception &e) {
        throw InternalGameException(std::string("FestivalTile::onLanded: ") + e.what());
    }
}
