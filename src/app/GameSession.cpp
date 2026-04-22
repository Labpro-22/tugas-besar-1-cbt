#include "app/GameSession.hpp"
#include "app/ExceptionHandler.hpp"
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
#include "models/Card/DemolitionCard.hpp"
#include "models/Card/DiscountCard.hpp"
#include "models/Card/LassoCard.hpp"
#include "models/Card/MoveCard.hpp"
#include "models/Card/ShieldCard.hpp"
#include "models/Card/SkillCard.hpp"
#include "models/Card/TeleportCard.hpp"
#include "models/GameManager/LogEntry.hpp"
#include "models/GameManager/Player.hpp"
#include "models/Property/Railroad.hpp"
#include "models/Property/Street.hpp"
#include "models/Property/Utility.hpp"


using namespace app;

GameSession::GameSession()
    : cli(),
      game(),
      board(),
      boardRenderer(),
      dice(),
      configuration("config"),
      queries(board, configuration, game),
      running(false),
      gameStarted(false),
      turnActionTaken(false),
      diceRolledThisTurn(false),
      gameOverAnnounced(false),
      gameOverReason(),
      winnerNames(),
      winnerCash(0),
      winnerPropertyCount(0),
      winnerCardCount(0),
      startupMode("MAIN_MENU"),
      startupPrompt(),
      startupExpectedPlayers(0),
      startupCollectedPlayers(0),
      snapshotCallback(),
      jailAttemptCounts(),
      skillCardFactory(),
      skillDeck(),
      skillDiscard() {}

void GameSession::setSnapshotCallback(SnapshotCallback callback) {
    snapshotCallback = std::move(callback);
}

void GameSession::requestStop() { running = false; }

void GameSession::run() {
    ExceptionHandler::guard("GameSession::run", [&]() {
        running = true;
        resetSessionState();

        if (!configuration.loadAllConfigs()) {
            const std::string detail = configuration.getLastError().empty()
                                           ? "Gagal membaca file konfigurasi."
                                           : configuration.getLastError();
            std::cerr << "Error: " << detail << "\n";
            std::cout << "Gagal membaca file konfigurasi pada folder config/.\n";
            std::cout << "Pastikan property.txt, railroad.txt, utility.txt, tax.txt, "
                         "special.txt, misc.txt, dan board.txt tersedia dan valid.\n";
            notifySnapshot();
            return;
        }

        prepareFreshBoard();
        updateStartupState("MAIN_MENU", "Pilih NEW GAME, LOAD GAME, atau EXIT.");
        notifySnapshot();

        if (!initializeGameFromMenu()) {
            notifySnapshot();
            return;
        }

        printWelcome();
        notifySnapshot();

        while (running) {
            if (gameStarted && game.isGameOver() && !gameOverAnnounced) {
                announceWinner();
                gameOverAnnounced = true;
                updateStartupState("GAME_OVER",
                                   "Permainan selesai. Gunakan NEW GAME, "
                                   "LOAD GAME, atau EXIT dari panel bawah.");
                notifySnapshot();
            }

            std::cout << "\n> ";
            Command command = cli.readCommand();
            if (command.name.empty()) {
                break;
            }

            handleCommand(command);
            notifySnapshot();
        }
    });
}

bool GameSession::initializeGameFromMenu() {
    printStartupMenu();
    updateStartupState("MAIN_MENU", "Pilih menu awal permainan.");
    notifySnapshot();

    while (running && !gameStarted) {
        updateStartupState("MAIN_MENU", "Pilih menu (0/1/2).");
        notifySnapshot();
        int choice = cli.getInputHandler().readChoice(
            0, 2, "Pilih menu (0/1/2): ");

        if (choice == 0) {
            running = false;
            return false;
        }

        if (choice == 1) {
            return initializeNewGame();
        }

        updateStartupState("LOAD_FILE", "Masukkan nama file save yang ingin dimuat.");
        notifySnapshot();
        std::string filename =
            cli.getInputHandler().readToken("Masukkan nama file save: ");
        if (initializeLoadedGame(filename)) {
            return true;
        }

        std::cout << "Gagal memuat file. Kembali ke menu awal.\n";
        printStartupMenu();
        updateStartupState("MAIN_MENU", "File gagal dimuat. Pilih menu awal lagi.");
        notifySnapshot();
    }

    return gameStarted;
}

bool GameSession::initializeNewGame() {
    resetSessionState();
    prepareFreshBoard();

    updateStartupState("PLAYER_COUNT", "Pilih jumlah pemain 2 sampai 4.");
    notifySnapshot();
    int playerCount =
        cli.getInputHandler().readChoice(2, 4, "Masukkan jumlah pemain (2-4): ");
    std::vector<std::string> usernames;
    usernames.reserve(static_cast<std::size_t>(playerCount));

    for (int i = 0; i < playerCount; ++i) {
        std::string username;
        do {
            updateStartupState("USERNAME",
                               "Masukkan username pemain " + std::to_string(i + 1) +
                                   " dari " + std::to_string(playerCount) + ".",
                               playerCount, i);
            notifySnapshot();
            username = trim(cli.getInputHandler().readPromptLine(
                "Masukkan username pemain " + std::to_string(i + 1) + ": ",
                "Username Pemain"));
        } while (username.empty());

        usernames.push_back(username);
    }

    initializePlayers(usernames);
    initializeSkillDeck();
    game.startNewGame();
    gameStarted = true;
    startTurn(true);
    turnActionTaken = false;
    updateStartupState("PLAYING", "", 0, 0);
    return true;
}

bool GameSession::initializeLoadedGame(const std::string& filename) {
    resetSessionState();
    prepareFreshBoard();
    gameStarted = loadFromFile(filename);
    turnActionTaken = false;
    if (gameStarted) {
        updateStartupState("PLAYING", "", 0, 0);
    }
    return gameStarted;
}

void GameSession::initializePlayers(const std::vector<std::string>& usernames) {
    std::vector<Player>& players = game.getPlayers();
    players.clear();
    players.reserve(usernames.size());

    for (const std::string& username : usernames) {
        players.emplace_back(username, configuration.getStartingCash());
    }

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(players.begin(), players.end(), generator);
}

void GameSession::prepareFreshBoard() {
    game.setConfiguration(configuration);
    game.setBoard(&board);
    game.setMaxTurn(configuration.getMaxTurn());
    board.initialize(configuration);
}

void GameSession::resetSessionState() {
    game.getPlayers().clear();
    game.getLogger().clear();
    game.setCurrentTurn(0);
    game.setActivePlayerIndex(0);
    jailAttemptCounts.clear();
    skillCardFactory.clear();
    skillDeck.clear();
    skillDiscard.clear();
    gameStarted = false;
    turnActionTaken = false;
    diceRolledThisTurn = false;
    gameOverAnnounced = false;
    gameOverReason.clear();
    winnerNames.clear();
    winnerCash = 0;
    winnerPropertyCount = 0;
    winnerCardCount = 0;
    startupMode = "MAIN_MENU";
    startupPrompt.clear();
    startupExpectedPlayers = 0;
    startupCollectedPlayers = 0;
}

void GameSession::printWelcome() const {
    std::cout << "========================================\n";
    std::cout << "          NIMONSPOLI ESTATE BOARD\n";
    std::cout << "========================================\n";
    std::cout << "Config dibaca dari folder: " << configuration.getConfigDir() << "\n";
    std::cout << "Jumlah petak aktif: " << board.getTileCount() << "\n";
    std::cout << "Command utama: CETAK_PAPAN, LEMPAR_DADU, ATUR_DADU X Y,\n";
    std::cout << "CETAK_AKTA, CETAK_PROPERTI, CETAK_LOG [N], GADAI, TEBUS,\n";
    std::cout << "BANGUN, GUNAKAN_KEMAMPUAN, SIMPAN nama_file.txt\n";
    std::cout << "========================================\n";
}

void GameSession::printStartupMenu() const {
    std::cout << "========================================\n";
    std::cout << "        NIMONSPOLI - START MENU\n";
    std::cout << "========================================\n";
    std::cout << "1. New Game\n";
    std::cout << "2. Load Game\n";
    std::cout << "0. Exit\n";
    std::cout << "========================================\n";
}

void GameSession::startTurn(bool drawSkillCard) {
    game.processTurn();
    diceRolledThisTurn = false;
    if (drawSkillCard) {
        awardSkillCardAtTurnStart();
    }
}

void GameSession::handleCommand(const Command& command) {
    const bool handled = ExceptionHandler::guard(
        "GameSession::handleCommand(" + command.name + ")", [&]() {
            if (command.name == "EXIT") {
                running = false;
                return;
            }
            if (command.name == "NEW_GAME") {
                if (gameStarted && !game.isGameOver()) {
                    cli.showError("NEW GAME hanya tersedia setelah permainan selesai.");
                    return;
                }
                std::cout << "Memulai permainan baru...\n";
                initializeNewGame();
                printWelcome();
                return;
            }
            if (command.name == "LOAD_GAME") {
                if (gameStarted && !game.isGameOver()) {
                    cli.showError("LOAD GAME hanya tersedia setelah permainan selesai.");
                    return;
                }
                const std::string filename =
                    command.args.empty()
                        ? cli.getInputHandler().readToken("Masukkan nama file save: ")
                        : command.args[0];
                if (initializeLoadedGame(filename)) {
                    std::cout << "Permainan berhasil dimuat dari: " << filename
                              << "\n";
                    printWelcome();
                } else {
                    cli.showError("Gagal memuat file save.");
                }
                return;
            }

            if (!gameStarted) {
                cli.showError("Permainan belum dimulai.");
                return;
            }
            if (game.isGameOver()) {
                cli.showError("Permainan sudah selesai. Pilih NEW GAME, LOAD GAME, "
                              "atau EXIT.");
                return;
            }

            if (command.name == "CETAK_PAPAN") {
                handlePrintBoard();
                return;
            }
            if (command.name == "LEMPAR_DADU") {
                handleRollDice(false);
                return;
            }
            if (command.name == "ATUR_DADU") {
                handleRollDice(true, std::stoi(command.args[0]),
                               std::stoi(command.args[1]));
                return;
            }
            if (command.name == "CETAK_AKTA") {
                handlePrintDeed();
                return;
            }
            if (command.name == "CETAK_PROPERTI") {
                handlePrintProperties();
                return;
            }
            if (command.name == "CETAK_LOG") {
                handlePrintLogs(command);
                return;
            }
            if (command.name == "GADAI") {
                handleMortgage();
                return;
            }
            if (command.name == "TEBUS") {
                handleRedeem();
                return;
            }
            if (command.name == "BANGUN") {
                handleBuild();
                return;
            }
            if (command.name == "GUNAKAN_KEMAMPUAN") {
                handleUseAbility();
                return;
            }
            if (command.name == "SIMPAN") {
                handleSave(command);
                return;
            }
            if (command.name == "MUAT") {
                handleLoad(command);
                return;
            }

            cli.showError("Command belum didukung di sesi GUI ini.");
        });
    if (!handled) {
        notifySnapshot();
    }
}

void GameSession::finishTurn() {
    if (game.isGameOver()) {
        return;
    }

    game.getCurrentPlayer().deactivateShield();
    dice.resetConsecutiveDoubles();
    game.advanceToNextPlayer();
    startTurn(true);
    turnActionTaken = false;
    std::cout << "\nGiliran berpindah ke " << game.getCurrentPlayer().getUsername()
              << ".\n";
}

void GameSession::announceWinner() {
    std::vector<Player*> winners;
    int bestCash = -1;
    int bestProperties = -1;
    int bestCards = -1;

    winnerNames.clear();
    winnerCash = 0;
    winnerPropertyCount = 0;
    winnerCardCount = 0;

    std::cout << "\nPermainan selesai!\n";
    if (game.getMaxTurn() > 0 && game.getCurrentTurn() > game.getMaxTurn()) {
        gameOverReason = "Batas maksimum giliran tercapai.";
    } else {
        gameOverReason = "Semua pemain kecuali pemenang bangkrut.";
    }

    std::cout << "Alasan: " << gameOverReason << "\n";

    std::cout << "\nRekap pemain:\n";
    for (Player& player : game.getPlayers()) {
        std::cout << "- " << player.getUsername()
                  << " | Status: " << playerStatusLabel(player)
                  << " | Uang: M" << player.getCash()
                  << " | Properti: " << player.getPropertyCount()
                  << " | Kartu: " << player.getCardCount() << "\n";

        if (player.getStatus() == BANKRUPT) {
            continue;
        }

        const int cash = player.getCash();
        const int properties = player.getPropertyCount();
        const int cards = player.getCardCount();
        const bool better =
            cash > bestCash ||
            (cash == bestCash && properties > bestProperties) ||
            (cash == bestCash && properties == bestProperties && cards > bestCards);
        const bool tied =
            cash == bestCash && properties == bestProperties && cards == bestCards;

        if (better) {
            winners.clear();
            winners.push_back(&player);
            bestCash = cash;
            bestProperties = properties;
            bestCards = cards;
        } else if (tied) {
            winners.push_back(&player);
        }
    }

    if (!winners.empty()) {
        winnerCash = winners.front()->getCash();
        winnerPropertyCount = winners.front()->getPropertyCount();
        winnerCardCount = winners.front()->getCardCount();
        for (Player* winner : winners) {
            if (winner != nullptr) {
                winnerNames.push_back(winner->getUsername());
            }
        }
    }

    cli.showWinner(winners);
}

void GameSession::updateStartupState(const std::string& mode,
                                     const std::string& prompt,
                                     int expectedPlayers,
                                     int collectedPlayers) {
    startupMode = mode;
    startupPrompt = prompt;
    startupExpectedPlayers = expectedPlayers;
    startupCollectedPlayers = collectedPlayers;
}

