#include "app/GameSession.hpp"
#include "app/GameSessionUtil.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "core/Board-Tiles/PropertyTile.hpp"
#include "core/Board-Tiles/Tile.hpp"
#include "models/Card/SkillCard.hpp"
#include "models/GameManager/LogEntry.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"

using namespace app;

void GameSession::notifySnapshot() {
    if (!snapshotCallback) {
        return;
    }

    snapshotCallback(buildSnapshot());
}

void GameSession::notifySnapshotImmediate() {
    notifySnapshot();
}

GameSnapshot GameSession::buildSnapshot() const {
    GameSnapshot snapshot;
    snapshot.gameStarted = gameStarted;
    snapshot.gameOver = gameStarted && game.isGameOver();
    snapshot.hasWinnerSummary = snapshot.gameOver && !winnerNames.empty();
    snapshot.currentTurn = game.getCurrentTurn();
    snapshot.maxTurn = game.getMaxTurn();
    snapshot.activePlayerIndex = game.getActivePlayerIndex();
    snapshot.startupMode = startupMode;
    snapshot.startupPrompt = startupPrompt;
    snapshot.startupExpectedPlayers = startupExpectedPlayers;
    snapshot.startupCollectedPlayers = startupCollectedPlayers;
    snapshot.die1 = dice.getDie1();
    snapshot.die2 = dice.getDie2();
    snapshot.hasDiceResult = dice.getDie1() > 0 && dice.getDie2() > 0;
    snapshot.headerText = buildHeaderText();
    snapshot.statusText = buildStatusText();
    snapshot.gameOverReason = gameOverReason;
    snapshot.winnerNames = winnerNames;
    snapshot.winnerCash = winnerCash;
    snapshot.winnerPropertyCount = winnerPropertyCount;
    snapshot.winnerCardCount = winnerCardCount;

    for (const Player& player : game.getPlayers()) {
        PlayerSnapshot playerSnapshot;
        playerSnapshot.name = player.getUsername();
        playerSnapshot.cash = player.getCash();
        playerSnapshot.position = player.getPosition();
        playerSnapshot.propertyCount = player.getPropertyCount();
        playerSnapshot.cardCount = player.getCardCount();
        playerSnapshot.active = player.getStatus() == ACTIVE;
        playerSnapshot.bankrupt = player.getStatus() == BANKRUPT;
        playerSnapshot.jailed = player.getStatus() == JAILED;
        playerSnapshot.detailText = buildPlayerDetailText(player);
        snapshot.players.push_back(playerSnapshot);
    }

    for (int i = 0; i < board.getTileCount(); ++i) {
        const Tile& tile = board.getTile(i);
        TileSnapshot tileSnapshot;
        tileSnapshot.position = i;
        const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
        tileSnapshot.code =
            i < static_cast<int>(layout.size()) ? layout[static_cast<std::size_t>(i)].code
                                                : tile.getCode();
        tileSnapshot.name = tile.getName();
        tileSnapshot.type = tile.getType();
        tileSnapshot.colorKey = resolveTileColorKey(tile);

        if (tile.getType() == "property") {
            const PropertyTile& propTile = static_cast<const PropertyTile&>(tile);
            const Property& property = propTile.getProperty();
            if (property.getOwner() != nullptr) {
                tileSnapshot.owner = property.getOwner()->getUsername();
            }
            tileSnapshot.buildingCount = property.getBuildingCount();
            tileSnapshot.mortgaged =
                property.getStatus() == PropertyStatus::MORTGAGED;
            tileSnapshot.festivalMultiplier = property.getFMult();
            tileSnapshot.festivalDuration = property.getFDur();
        }

        snapshot.tiles.push_back(tileSnapshot);
    }

    return snapshot;
}

std::string GameSession::buildHeaderText() const {
    if (!gameStarted) {
        return "The Grand Tactician: Nimonspoli";
    }

    std::ostringstream oss;
    oss << "Estate Ledger   The Board   Cartography   Archives";
    return oss.str();
}

std::string GameSession::buildStatusText() const {
    std::ostringstream oss;

    if (!gameStarted) {
        oss << "Konfigurasi: " << configuration.getConfigDir() << "\n";
        if (configuration.isConfigLoaded()) {
            oss << "Board tiles : " << board.getTileCount() << "\n";
            oss << "Go salary   : M" << configuration.getGoSalary() << "\n";
            oss << "Jail fine   : M" << configuration.getJailFine() << "\n";
            oss << "Max turn    : " << configuration.getMaxTurn() << "\n";
            oss << "Saldo awal  : M" << configuration.getStartingCash() << "\n";
        }
        oss << "\nPapan dinamis aktif.\n";
        if (!startupPrompt.empty()) {
            oss << startupPrompt << "\n";
        }
        if (startupMode == "PLAYER_COUNT") {
            oss << "Gunakan tombol 2 / 3 / 4 pemain, atau ketik angkanya.\n";
        } else if (startupMode == "USERNAME") {
            oss << "Username terisi : " << startupCollectedPlayers << " / "
                << startupExpectedPlayers << "\n";
            oss << "Masukkan nama pemain lewat pop-up.\n";
        } else if (startupMode == "LOAD_FILE") {
            oss << "Masukkan nama file save lewat pop-up.\n";
        } else {
            oss << "Pilih NEW GAME atau LOAD GAME dari panel bawah.\n";
        }
        return oss.str();
    }

    if (!game.getPlayers().empty()) {
        oss << buildPlayerDetailText(game.getCurrentPlayer());
    }

    return oss.str();
}

std::string GameSession::buildPlayerDetailText(const Player& player) const {
    std::ostringstream oss;

    if (board.getTileCount() <= 0) {
        return "";
    }

    const int currentPosition =
        std::clamp(player.getPosition(), 0, board.getTileCount() - 1);
    const Tile& currentTile = board.getTile(currentPosition);
    const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
    const std::string tileCode =
        currentPosition >= 0 && currentPosition < static_cast<int>(layout.size())
            ? layout[static_cast<std::size_t>(currentPosition)].code
            : currentTile.getCode();

    oss << "Estate Holder : " << player.getUsername() << "\n";
    oss << "Status        : " << playerStatusLabel(player) << "\n";
    oss << "Turn          : " << game.getCurrentTurn() << " / "
        << game.getMaxTurn() << "\n";
    oss << "Cash          : M" << player.getCash() << "\n";
    oss << "Properti      : " << player.getPropertyCount() << "\n";
    oss << "Kartu         : " << player.getCardCount() << "\n";
    oss << "Posisi        : " << tileCode << " - " << currentTile.getName()
        << "\n";
    if (player.hasShieldActive()) {
        oss << "Shield        : AKTIF\n";
    }
    if (player.hasDiscount()) {
        oss << "Discount      : " << player.getDiscountPercentage()
            << "%\n";
    }

    oss << "\nPROPERTY PORTFOLIO\n";
    if (player.getProperties().empty()) {
        oss << "- Belum memiliki properti.\n";
    } else {
        int displayed = 0;
        for (Property* property : player.getProperties()) {
            if (property == nullptr) {
                continue;
            }

            oss << "- " << property->getCode() << " | " << property->getName();
            if (property->getStatus() == PropertyStatus::MORTGAGED) {
                oss << " [M]";
            }

            oss << " | " << buildingLabel(property);
            if (property->getFMult() > 1) {
                oss << " | Festival x" << property->getFMult() << " ("
                    << property->getFDur() << "t)";
            }
            oss << "\n";

            displayed++;
            if (displayed >= 12 &&
                player.getProperties().size() >
                    static_cast<std::size_t>(displayed)) {
                oss << "... +"
                    << (player.getProperties().size() -
                        static_cast<std::size_t>(displayed))
                    << " properti lain\n";
                break;
            }
        }
    }

    oss << "\nKARTU KEMAMPUAN\n";
    if (player.getHand().empty()) {
        oss << "- Tidak ada kartu di tangan.\n";
    } else {
        for (Card* card : player.getHand()) {
            if (card == nullptr) {
                continue;
            }
            oss << "- " << skillCardDisplayLabel(card) << " | "
                << card->getDescription() << "\n";
        }
    }

    return oss.str();
}

