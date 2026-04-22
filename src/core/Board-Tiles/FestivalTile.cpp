#include "core/Board-Tiles/FestivalTile.hpp"

#include "core/Board-Tiles/Board.hpp"
#include "core/Board-Tiles/PropertyTile.hpp"
#include "models/GameManager/GameManager.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Property.hpp"
#include "views/InputHandler.hpp"
#include "exception/NimonspoliExceptions.hpp"

#include <sstream>
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
            logTileEvent(game, player, "FESTIVAL",
                         "Mendarat di Festival, tetapi belum memiliki properti.");
            return;
        }

        std::ostringstream propertyList;
        propertyList << "Mendarat di Festival. Properti yang dapat dipilih: ";
        bool firstProperty = true;
        for (PropertyTile *tile : ownedTiles) {
            if (tile == nullptr) {
                continue;
            }

            if (!firstProperty) {
                propertyList << ", ";
            }
            propertyList << tile->getProperty().getCode() << " ("
                         << tile->getProperty().getName() << ")";
            firstProperty = false;
        }
        logTileEvent(game, player, "FESTIVAL", propertyList.str());
        game.pushSnapshot();

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
                logTileEvent(game, player, "FESTIVAL",
                             "Kode properti festival tidak valid: " +
                                 propertyCode);
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

        std::string festivalState = "Efek festival aktif.";
        if (nextMultiplier == currentMultiplier) {
            festivalState =
                "Efek festival sudah maksimum; harga sewa sudah digandakan tiga kali.";
        } else if (currentMultiplier > 1) {
            festivalState = "Efek festival diperkuat.";
        }

        const int baseRent = selectedTile->getProperty().getPropertyDetail();
        if (baseRent < 0) {
            throw PropertyException("Nilai sewa properti tidak valid.",
                                    "PROPERTY_RENT_ERROR");
        }

        selectedTile->applyFestivalEffect(nextMultiplier, 3);
        game.executeFestival(player, selectedTile->getProperty().getCode());

        std::ostringstream detail;
        detail << festivalState << " Properti "
               << selectedTile->getProperty().getCode() << " ("
               << selectedTile->getProperty().getName() << "). ";
        if (currentMultiplier > 1) {
            detail << "Sewa sebelumnya M" << (baseRent * currentMultiplier)
                   << ". ";
        } else {
            detail << "Sewa awal M" << baseRent << ". ";
        }
        detail << "Sewa sekarang M" << (baseRent * nextMultiplier)
               << ". Durasi 3 giliran.";
        logTileEvent(game, player, "FESTIVAL", detail.str());
    } catch (const NimonspoliException &) {
        throw;
    } catch (const std::exception &e) {
        throw InternalGameException(std::string("FestivalTile::onLanded: ") + e.what());
    }
}
