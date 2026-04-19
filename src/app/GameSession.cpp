#include "GameSession.hpp"

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

#include "../core/Board-Tiles/PropertyTile.hpp"
#include "../core/Board-Tiles/Tile.hpp"
#include "../models/Card/DemolitionCard.hpp"
#include "../models/Card/DiscountCard.hpp"
#include "../models/Card/LassoCard.hpp"
#include "../models/Card/MoveCard.hpp"
#include "../models/Card/ShieldCard.hpp"
#include "../models/Card/SkillCard.hpp"
#include "../models/Card/TeleportCard.hpp"
#include "../models/GameManager/LogEntry.hpp"
#include "../models/GameManager/Player.hpp"
#include "../models/Property/Railroad.hpp"
#include "../models/Property/Street.hpp"
#include "../models/Property/Utility.hpp"

namespace {

std::string uppercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return text;
}

std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string playerStatusLabel(const Player& player) {
    switch (player.getStatus()) {
        case ACTIVE:
            return "ACTIVE";
        case BANKRUPT:
            return "BANKRUPT";
        case JAILED:
            return "JAILED";
    }

    return "UNKNOWN";
}

std::string colorGroupKey(ColorGroup color) {
    switch (color) {
        case ColorGroup::COKLAT:
            return "CK";
        case ColorGroup::BIRU_MUDA:
            return "BM";
        case ColorGroup::MERAH_MUDA:
            return "PK";
        case ColorGroup::ORANGE:
            return "OR";
        case ColorGroup::MERAH:
            return "MR";
        case ColorGroup::KUNING:
            return "KN";
        case ColorGroup::HIJAU:
            return "HJ";
        case ColorGroup::BIRU_TUA:
            return "BT";
        case ColorGroup::ABU_ABU:
            return "AB";
    }

    return "DF";
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

    return "DEFAULT";
}

std::string buildingLabel(const Street* street) {
    if (street == nullptr) {
        return "-";
    }

    const int buildingCount = street->getBuildingCount();
    if (buildingCount <= 0) {
        return "0 rumah";
    }
    if (buildingCount >= static_cast<int>(BuildingLevel::HOTEL)) {
        return "Hotel";
    }

    return std::to_string(buildingCount) + " rumah";
}

std::string trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }

    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

std::vector<std::string> tokenize(const std::string& line) {
    std::stringstream ss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int randomInt(int min, int max) {
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

std::string skillCardDisplayLabel(const Card* card) {
    if (card == nullptr) {
        return "-";
    }

    std::ostringstream oss;
    oss << card->getType();
    if (card->getValue() != 0) {
        oss << " (" << card->getValue() << ")";
    }
    return oss.str();
}

std::string resolveTileColorKey(const Tile& tile) {
    const PropertyTile* propertyTile = dynamic_cast<const PropertyTile*>(&tile);
    if (propertyTile != nullptr) {
        const Property& property = propertyTile->getProperty();
        const Street* street = dynamic_cast<const Street*>(&property);
        if (street != nullptr) {
            return colorGroupKey(street->getColorGroup());
        }
        if (property.getType() == "Utility") {
            return "AB";
        }
        if (property.getType() == "Railroad") {
            return "DF";
        }
    }

    if (tile.getType() == "festival") {
        return "FV";
    }
    return "DF";
}

}  // namespace

GameSession::GameSession()
    : cli(),
      game(),
      board(),
      boardRenderer(),
      dice(),
      configuration("config"),
      running(false),
      gameStarted(false),
      turnActionTaken(false),
      diceRolledThisTurn(false),
      startupMode("MAIN_MENU"),
      startupPrompt(),
      startupExpectedPlayers(0),
      startupCollectedPlayers(0),
      snapshotCallback(),
      jailAttemptCounts(),
      ownedSkillCards(),
      skillDeck(),
      skillDiscard() {}

void GameSession::setSnapshotCallback(SnapshotCallback callback) {
    snapshotCallback = std::move(callback);
}

void GameSession::requestStop() { running = false; }

void GameSession::run() {
    running = true;
    resetSessionState();

    if (!configuration.loadAllConfigs()) {
        std::cout << "Gagal membaca file konfigurasi pada folder config/.\n";
        std::cout << "Pastikan property.txt, railroad.txt, utility.txt, tax.txt, "
                     "special.txt, misc.txt, dan board.txt tersedia.\n";
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
        if (gameStarted && game.isGameOver()) {
            announceWinner();
            notifySnapshot();
            break;
        }

        std::cout << "\n> ";
        Command command = cli.readCommand();
        if (command.name.empty()) {
            break;
        }

        handleCommand(command);
        notifySnapshot();
    }
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
    ownedSkillCards.clear();
    skillDeck.clear();
    skillDiscard.clear();
    gameStarted = false;
    turnActionTaken = false;
    diceRolledThisTurn = false;
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

void GameSession::initializeSkillDeck() {
    skillDeck.clear();
    skillDiscard.clear();

    skillDeck.insert(skillDeck.end(), 4, "MoveCard");
    skillDeck.insert(skillDeck.end(), 3, "DiscountCard");
    skillDeck.insert(skillDeck.end(), 2, "ShieldCard");
    skillDeck.insert(skillDeck.end(), 2, "TeleportCard");
    skillDeck.insert(skillDeck.end(), 2, "LassoCard");
    skillDeck.insert(skillDeck.end(), 2, "DemolitionCard");
    shuffleSkillDeck();
}

void GameSession::shuffleSkillDeck() {
    std::shuffle(skillDeck.begin(), skillDeck.end(),
                 std::mt19937(std::random_device{}()));
}

void GameSession::ensureSkillDeckAvailable() {
    if (!skillDeck.empty() || skillDiscard.empty()) {
        return;
    }

    skillDeck = skillDiscard;
    skillDiscard.clear();
    shuffleSkillDeck();
    std::cout << "Deck kartu kemampuan habis. Mencampur ulang discard pile...\n";
}

SkillCard* GameSession::createSkillCardInstance(const std::string& type,
                                                int value,
                                                int duration) {
    const std::string normalized = uppercase(type);
    std::unique_ptr<SkillCard> card;

    if (normalized == "MOVECARD") {
        const int steps = value > 0 ? value : randomInt(1, 6);
        card = std::make_unique<MoveCard>(0, steps);
    } else if (normalized == "DISCOUNTCARD") {
        const int discount = value > 0 ? value : (randomInt(1, 5) * 10);
        const int effectiveDuration = duration > 0 ? duration : 1;
        card = std::make_unique<DiscountCard>(0, discount, effectiveDuration);
    } else if (normalized == "SHIELDCARD") {
        card = std::make_unique<ShieldCard>();
    } else if (normalized == "TELEPORTCARD") {
        card = std::make_unique<TeleportCard>();
    } else if (normalized == "LASSOCARD") {
        card = std::make_unique<LassoCard>();
    } else if (normalized == "DEMOLITIONCARD") {
        card = std::make_unique<DemolitionCard>();
    }

    if (!card) {
        return nullptr;
    }

    SkillCard* rawCard = card.get();
    ownedSkillCards.push_back(std::move(card));
    return rawCard;
}

SkillCard* GameSession::drawSkillCard() {
    ensureSkillDeckAvailable();
    if (skillDeck.empty()) {
        return nullptr;
    }

    const std::string type = skillDeck.back();
    skillDeck.pop_back();
    return createSkillCardInstance(type);
}

void GameSession::awardSkillCardAtTurnStart() {
    if (!gameStarted || game.getPlayers().empty()) {
        return;
    }

    Player& player = game.getCurrentPlayer();
    SkillCard* drawnCard = drawSkillCard();
    if (drawnCard == nullptr) {
        std::cout << "Tidak ada kartu kemampuan yang tersedia untuk dibagikan.\n";
        return;
    }

    player.addCard(drawnCard);
    std::cout << "Kamu mendapatkan 1 kartu acak baru!\n";
    std::cout << "Kartu yang didapat: " << drawnCard->getType();
    if (drawnCard->getValue() != 0) {
        std::cout << " (" << drawnCard->getValue() << ")";
    }
    std::cout << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "Mendapat " + skillCardDisplayLabel(drawnCard));

    if (player.getCardCount() <= 3) {
        return;
    }

    std::cout << "PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (maksimal 3).\n";
    std::cout << "Kamu diwajibkan membuang 1 kartu.\n";
    std::cout << "Daftar Kartu Kemampuan Anda:\n";
    for (std::size_t i = 0; i < player.getHand().size(); ++i) {
        Card* card = player.getHand()[i];
        std::cout << (i + 1) << ". " << card->getType() << " - "
                  << card->getDescription() << "\n";
    }

    const int discardChoice = cli.getInputHandler().readChoice(
        1, static_cast<int>(player.getHand().size()),
        "Pilih nomor kartu yang ingin dibuang: ");
    SkillCard* discardCard =
        dynamic_cast<SkillCard*>(player.getHand()[static_cast<std::size_t>(discardChoice - 1)]);
    if (discardCard != nullptr) {
        std::cout << discardCard->getType() << " telah dibuang.\n";
        discardSkillCard(player, discardCard);
    }
}

void GameSession::handleCommand(const Command& command) {
    if (!gameStarted) {
        cli.showError("Permainan belum dimulai.");
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
        handleRollDice(true, std::stoi(command.args[0]), std::stoi(command.args[1]));
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
}

void GameSession::handlePrintBoard() {
    std::vector<Player*> playerPointers;
    playerPointers.reserve(game.getPlayers().size());
    for (Player& player : game.getPlayers()) {
        playerPointers.push_back(&player);
    }

    std::cout << boardRenderer.renderBoard(&board, playerPointers,
                                           game.getCurrentTurn(),
                                           game.getMaxTurn());
}

void GameSession::handleRollDice(bool manual, int d1, int d2) {
    Player& currentPlayer = game.getCurrentPlayer();
    const bool startedTurnInJail = currentPlayer.getStatus() == JAILED;
    int jailAttempts = 0;
    bool releasedFromJailByDouble = false;

    if (startedTurnInJail) {
        const int jailFine = configuration.getJailFine();
        jailAttempts = getJailAttemptCount(currentPlayer);

        std::cout << currentPlayer.getUsername() << " sedang berada di Penjara.\n";

        if (jailAttempts >= 3) {
            std::cout << "Ini adalah giliran ke-4 di penjara. "
                      << currentPlayer.getUsername()
                      << " wajib membayar denda M" << jailFine
                      << " sebelum melempar dadu.\n";
            game.executeTaxPayment(currentPlayer, jailFine, true);
            if (currentPlayer.getStatus() == BANKRUPT) {
                turnActionTaken = true;
                finishTurn();
                return;
            }

            releasePlayerFromJail(currentPlayer);
            std::cout << currentPlayer.getUsername()
                      << " berhasil keluar dari Penjara setelah membayar denda.\n";
        } else {
            std::cout << "Pilih cara keluar:\n";
            std::cout << "1. Bayar denda M" << jailFine << "\n";
            std::cout << "2. Coba lempar dadu (harus double)\n";

            const int choice = cli.getInputHandler().readChoice(
                1, 2, "Pilihan (1/2): ");
            if (choice == 1) {
                game.executeTaxPayment(currentPlayer, jailFine, true);
                if (currentPlayer.getStatus() == BANKRUPT) {
                    turnActionTaken = true;
                    finishTurn();
                    return;
                }

                releasePlayerFromJail(currentPlayer);
                std::cout << currentPlayer.getUsername()
                          << " keluar dari Penjara setelah membayar denda.\n";
            }
        }
    }

    if (manual) {
        if (d1 < 1 || d1 > 6 || d2 < 1 || d2 > 6) {
            cli.showError("Nilai dadu harus di antara 1 sampai 6.");
            return;
        }

        dice.setValues(d1, d2);
        std::cout << "Dadu diatur secara manual.\n";
    } else {
        std::cout << "Mengocok dadu...\n";
        dice.roll();
    }

    diceRolledThisTurn = true;

    const int total = dice.getTotal();
    const bool rolledDouble = dice.checkDouble();

    game.getLogger().log(
        game.getCurrentTurn(), currentPlayer.getUsername(), "DADU",
        "Lempar: " + std::to_string(dice.getDie1()) + "+" +
            std::to_string(dice.getDie2()) + "=" + std::to_string(total));

    std::cout << "Hasil: " << dice.getDie1() << " + " << dice.getDie2() << " = "
              << total << "\n";

    if (startedTurnInJail && currentPlayer.getStatus() == JAILED) {
        if (!rolledDouble) {
            jailAttemptCounts[currentPlayer.getUsername()] = jailAttempts + 1;
            std::cout << "Kamu gagal mendapatkan double dan tetap berada di Penjara.\n";
            std::cout << "Percobaan keluar penjara: "
                      << jailAttemptCounts[currentPlayer.getUsername()]
                      << " / 3\n";
            game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                                 "PENJARA",
                                 "Gagal keluar dari penjara (percobaan " +
                                     std::to_string(
                                         jailAttemptCounts[currentPlayer.getUsername()]) +
                                     "/3)");
            notifySnapshotImmediate();
            turnActionTaken = true;
            finishTurn();
            return;
        }

        releasePlayerFromJail(currentPlayer);
        releasedFromJailByDouble = true;
        std::cout << "Double! " << currentPlayer.getUsername()
                  << " berhasil keluar dari Penjara.\n";
        game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                             "PENJARA", "Keluar dari penjara karena double");
    }

    if (dice.hasThreeConsecutiveDoubles()) {
        std::cout << "Double 3 kali berturut-turut! "
                  << currentPlayer.getUsername()
                  << " langsung dikirim ke Penjara.\n";
        game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                             "DOUBLE",
                             "Double ke-3 berturut-turut -> masuk penjara");
        game.goToJail(currentPlayer);
        markPlayerJailed(currentPlayer);
        notifySnapshotImmediate();
        turnActionTaken = true;
        finishTurn();
        return;
    }

    std::cout << "Memajukan Bidak " << currentPlayer.getUsername() << " sebanyak "
              << total << " petak...\n";

    game.moveCurrentPlayer(total);
    Tile& tile = board.getTile(currentPlayer.getPosition());
    std::cout << "Bidak mendarat di: " << tile.getName() << ".\n";
    notifySnapshotImmediate();
    tile.onLanded(currentPlayer, game);
    if (currentPlayer.getStatus() == JAILED) {
        markPlayerJailed(currentPlayer);
    }

    turnActionTaken = true;

    if (rolledDouble && currentPlayer.getStatus() == ACTIVE &&
        !releasedFromJailByDouble && !game.isGameOver()) {
        std::cout << currentPlayer.getUsername()
                  << " mendapat double. Giliran tambahan!\n";
        game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                             "DOUBLE", "Mendapat giliran tambahan");
        notifySnapshotImmediate();
        return;
    }

    finishTurn();
}

void GameSession::markPlayerJailed(Player& player) {
    if (player.getStatus() == JAILED) {
        jailAttemptCounts[player.getUsername()] = 0;
    }
}

void GameSession::releasePlayerFromJail(Player& player) {
    player.setActive();
    jailAttemptCounts.erase(player.getUsername());
}

int GameSession::getJailAttemptCount(const Player& player) const {
    const auto it = jailAttemptCounts.find(player.getUsername());
    if (it == jailAttemptCounts.end()) {
        return 0;
    }

    return it->second;
}

void GameSession::handlePrintDeed() {
    std::string code =
        uppercase(cli.getInputHandler().readToken("Masukkan kode petak: "));

    Property* property = findPropertyByCode(code);
    if (property == nullptr) {
        std::cout << "Petak \"" << code
                  << "\" tidak ditemukan atau bukan properti.\n";
        return;
    }

    cli.getPropertyView().showDeed(property);
}

void GameSession::handlePrintProperties() {
    cli.getPropertyView().showPlayerProperties(&game.getCurrentPlayer());
}

void GameSession::handlePrintLogs(const Command& command) {
    std::vector<LogEntry> entries = game.getLogger().getEntries();
    if (command.args.empty()) {
        cli.getLogView().showLogs(entries.data(), entries.size());
        return;
    }

    cli.getLogView().showLastLogs(entries.data(), entries.size(),
                                  std::stoi(command.args[0]));
}

void GameSession::handleMortgage() {
    Player& currentPlayer = game.getCurrentPlayer();
    std::vector<Property*> mortgageable = getMortgageableProperties();
    cli.getPropertyView().showMortgageOptions(mortgageable);
    if (mortgageable.empty()) {
        return;
    }

    int choice = cli.getInputHandler().readChoice(
        0, static_cast<int>(mortgageable.size()),
        "Pilih nomor properti (0 untuk batal): ");
    if (choice == 0) {
        return;
    }

    Property* property = mortgageable[static_cast<std::size_t>(choice - 1)];
    const int received = property->mortgage();
    currentPlayer.addCash(received);

    std::cout << property->getName() << " berhasil digadaikan.\n";
    std::cout << "Kamu menerima M" << received << " dari Bank.\n";
    std::cout << "Uang kamu sekarang: M" << currentPlayer.getCash() << "\n";

    game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                         "GADAI", property->getCode() + " digadaikan");
    turnActionTaken = true;
}

void GameSession::handleRedeem() {
    Player& currentPlayer = game.getCurrentPlayer();
    std::vector<Property*> redeemable = getRedeemableProperties();
    cli.getPropertyView().showRedeemOptions(redeemable);
    if (redeemable.empty()) {
        return;
    }

    int choice = cli.getInputHandler().readChoice(
        0, static_cast<int>(redeemable.size()),
        "Pilih nomor properti (0 untuk batal): ");
    if (choice == 0) {
        return;
    }

    Property* property = redeemable[static_cast<std::size_t>(choice - 1)];
    const int baseRedeemPrice = property->getBuyPrice();
    const int redeemPrice = applyDiscountToAmount(currentPlayer, baseRedeemPrice);
    if (!currentPlayer.canPay(redeemPrice)) {
        std::cout << "Uang kamu tidak cukup untuk menebus "
                  << property->getName() << ".\n";
        std::cout << "Harga tebus: M" << redeemPrice << " | Uang kamu: M"
                  << currentPlayer.getCash() << "\n";
        return;
    }

    currentPlayer.reduceCash(redeemPrice);
    property->redeem();

    std::cout << property->getName() << " berhasil ditebus!\n";
    std::cout << "Kamu membayar M" << baseRedeemPrice;
    if (redeemPrice != baseRedeemPrice) {
        std::cout << " -> M" << redeemPrice << " setelah diskon";
    }
    std::cout << " ke Bank.\n";
    std::cout << "Uang kamu sekarang: M" << currentPlayer.getCash() << "\n";

    game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                         "TEBUS", property->getCode() + " ditebus");
    turnActionTaken = true;
}

void GameSession::handleBuild() {
    Player& currentPlayer = game.getCurrentPlayer();
    std::map<std::string, std::vector<Property*>> buildableGroups;
    std::map<std::string, std::vector<Street*>> groupedStreets;

    for (Street* street : getBuildableStreets()) {
        if (street == nullptr) {
            continue;
        }

        const std::string label = colorGroupLabel(street->getColorGroup());
        buildableGroups[label].push_back(street);
        groupedStreets[label].push_back(street);
    }

    cli.getPropertyView().showBuildOptions(buildableGroups);
    if (buildableGroups.empty()) {
        return;
    }

    std::vector<std::string> orderedGroups;
    for (const auto& entry : buildableGroups) {
        orderedGroups.push_back(entry.first);
    }

    int groupChoice = cli.getInputHandler().readChoice(
        0, static_cast<int>(orderedGroups.size()),
        "Pilih nomor color group (0 untuk batal): ");
    if (groupChoice == 0) {
        return;
    }

    std::vector<Street*>& streets =
        groupedStreets[orderedGroups[static_cast<std::size_t>(groupChoice - 1)]];

    int minimumBuilding = streets.front()->getBuildingCount();
    bool allReadyForHotel = true;
    for (Street* street : streets) {
        minimumBuilding = std::min(minimumBuilding, street->getBuildingCount());
        if (street->getBuildingCount() <
            static_cast<int>(BuildingLevel::FOUR_HOUSE)) {
            allReadyForHotel = false;
        }
    }

    std::vector<Street*> eligible;
    std::cout << "Color group ["
              << orderedGroups[static_cast<std::size_t>(groupChoice - 1)] << "]:\n";
    for (Street* street : streets) {
        const bool canBuild =
            allReadyForHotel
                ? street->getBuildingCount() ==
                      static_cast<int>(BuildingLevel::FOUR_HOUSE)
                : street->getBuildingCount() == minimumBuilding &&
                      street->getBuildingCount() <
                          static_cast<int>(BuildingLevel::FOUR_HOUSE);

        std::cout << "- " << street->getName() << " (" << street->getCode()
                  << ") : " << buildingLabel(street);
        if (canBuild) {
            std::cout << " <- dapat dibangun";
            eligible.push_back(street);
        }
        std::cout << "\n";
    }

    if (eligible.empty()) {
        std::cout << "Tidak ada petak yang bisa dibangun saat ini.\n";
        return;
    }

    int propertyChoice = cli.getInputHandler().readChoice(
        0, static_cast<int>(eligible.size()), "Pilih petak (0 untuk batal): ");
    if (propertyChoice == 0) {
        return;
    }

    Street* target = eligible[static_cast<std::size_t>(propertyChoice - 1)];
    const bool hotelUpgrade =
        target->getBuildingCount() == static_cast<int>(BuildingLevel::FOUR_HOUSE);
    const int baseCost =
        hotelUpgrade ? target->getHotelCost() : target->getHouseCost();
    const int cost = applyDiscountToAmount(currentPlayer, baseCost);

    if (!currentPlayer.canPay(cost)) {
        std::cout << "Uang kamu tidak cukup. Biaya: M" << cost
                  << " | Uang kamu: M" << currentPlayer.getCash() << "\n";
        return;
    }

    currentPlayer.reduceCash(cost);
    target->setBuildingCount(target->getBuildingCount() + 1);

    if (hotelUpgrade) {
        std::cout << target->getName() << " di-upgrade ke Hotel! Biaya: M"
                  << baseCost;
        if (cost != baseCost) {
            std::cout << " -> M" << cost << " setelah diskon";
        }
        std::cout << "\n";
    } else {
        std::cout << "Kamu membangun 1 rumah di " << target->getName()
                  << ". Biaya: M" << baseCost;
        if (cost != baseCost) {
            std::cout << " -> M" << cost << " setelah diskon";
        }
        std::cout << "\n";
    }
    std::cout << "Uang tersisa: M" << currentPlayer.getCash() << "\n";

    game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                         "BANGUN",
                         target->getCode() + " -> " + buildingLabel(target));
    turnActionTaken = true;
}

void GameSession::handleUseAbility() {
    Player& currentPlayer = game.getCurrentPlayer();
    std::vector<Card*>& hand = currentPlayer.getHand();

    if (hand.empty()) {
        std::cout << "Kamu tidak memiliki kartu kemampuan.\n";
        return;
    }
    if (currentPlayer.getStatus() == JAILED) {
        std::cout << "Kartu kemampuan tidak dapat digunakan saat berada di Penjara.\n";
        return;
    }
    if (diceRolledThisTurn) {
        std::cout << "Kartu kemampuan hanya bisa digunakan SEBELUM melempar dadu.\n";
        return;
    }
    if (!currentPlayer.canUseAbility()) {
        std::cout
            << "Kamu sudah menggunakan kartu kemampuan pada giliran ini!\n";
        return;
    }

    std::cout << "Daftar Kartu Kemampuan Spesial Anda:\n";
    for (std::size_t i = 0; i < hand.size(); ++i) {
        std::cout << (i + 1) << ". " << hand[i]->getType() << " - "
                  << hand[i]->getDescription() << "\n";
    }
    std::cout << "0. Batal\n";

    int choice = cli.getInputHandler().readChoice(
        0, static_cast<int>(hand.size()),
        "Pilih kartu yang ingin digunakan: ");
    if (choice == 0) {
        return;
    }

    SkillCard* selected =
        dynamic_cast<SkillCard*>(hand[static_cast<std::size_t>(choice - 1)]);
    if (selected == nullptr) {
        std::cout << "Kartu terpilih tidak valid.\n";
        return;
    }

    if (!executeSkillCard(currentPlayer, selected)) {
        return;
    }

    selected->markAsUsed();
    currentPlayer.setUsedAbility();
    discardSkillCard(currentPlayer, selected);
    turnActionTaken = true;

    if (currentPlayer.getStatus() == JAILED || currentPlayer.getStatus() == BANKRUPT) {
        finishTurn();
    }
}

void GameSession::discardSkillCard(Player& player, SkillCard* card) {
    if (card == nullptr) {
        return;
    }

    skillDiscard.push_back(card->getType());
    player.removeCard(card);
    ownedSkillCards.erase(
        std::remove_if(ownedSkillCards.begin(), ownedSkillCards.end(),
                       [card](const std::unique_ptr<SkillCard>& ownedCard) {
                           return ownedCard.get() == card;
                       }),
        ownedSkillCards.end());
}

bool GameSession::executeSkillCard(Player& player, SkillCard* card) {
    if (card == nullptr) {
        return false;
    }

    if (dynamic_cast<MoveCard*>(card) != nullptr) {
        return executeMoveCard(player, *card);
    }
    if (dynamic_cast<DiscountCard*>(card) != nullptr) {
        return executeDiscountCard(player, *card);
    }
    if (dynamic_cast<ShieldCard*>(card) != nullptr) {
        return executeShieldCard(player, *card);
    }
    if (dynamic_cast<TeleportCard*>(card) != nullptr) {
        return executeTeleportCard(player, *card);
    }
    if (dynamic_cast<LassoCard*>(card) != nullptr) {
        return executeLassoCard(player, *card);
    }
    if (dynamic_cast<DemolitionCard*>(card) != nullptr) {
        return executeDemolitionCard(player, *card);
    }

    std::cout << "Kartu kemampuan belum didukung.\n";
    return false;
}

bool GameSession::executeMoveCard(Player& player, SkillCard& card) {
    MoveCard* moveCard = dynamic_cast<MoveCard*>(&card);
    if (moveCard == nullptr) {
        return false;
    }

    const int steps = moveCard->getSteps();
    std::cout << "MoveCard diaktifkan! Bergerak maju " << steps << " petak.\n";
    game.moveCurrentPlayer(steps);
    std::cout << "Bidak mendarat di: "
              << board.getTile(player.getPosition()).getName() << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "MoveCard -> maju " + std::to_string(steps) + " petak");
    resolveLandingAfterAbility(player, true);
    return true;
}

bool GameSession::executeDiscountCard(Player& player, SkillCard& card) {
    DiscountCard* discountCard = dynamic_cast<DiscountCard*>(&card);
    if (discountCard == nullptr) {
        return false;
    }

    player.applyDiscount(discountCard->getDiscountPercent(),
                         discountCard->getRemainingDuration());
    std::cout << "DiscountCard diaktifkan! Diskon "
              << discountCard->getDiscountPercent()
              << "% aktif untuk giliran ini.\n";
    game.getLogger().log(
        game.getCurrentTurn(), player.getUsername(), "KARTU",
        "DiscountCard aktif " + std::to_string(discountCard->getDiscountPercent()) +
            "%");
    return true;
}

bool GameSession::executeShieldCard(Player& player, SkillCard&) {
    player.activateShield();
    std::cout << "ShieldCard diaktifkan! Kamu kebal terhadap tagihan atau sanksi"
                 " selama giliran ini.\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "ShieldCard aktif");
    return true;
}

bool GameSession::executeTeleportCard(Player& player, SkillCard&) {
    int targetPosition = -1;
    while (targetPosition < 0 || targetPosition >= board.getTileCount()) {
        std::string token = uppercase(trim(cli.getInputHandler().readPromptLine(
            "Masukkan kode petak tujuan teleport: ", "Teleport")));
        const bool numeric =
            !token.empty() &&
            std::all_of(token.begin(), token.end(), [](unsigned char ch) {
                return std::isdigit(ch) != 0;
            });

        targetPosition = numeric ? std::stoi(token) : findTilePositionByCode(token);
        if (targetPosition < 0 || targetPosition >= board.getTileCount()) {
            std::cout << "Kode petak tidak valid.\n";
        }
    }

    player.setPosition(targetPosition);
    std::cout << "TeleportCard diaktifkan! Bidak dipindahkan ke "
              << board.getTile(targetPosition).getName() << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "TeleportCard -> " + board.getTile(targetPosition).getCode());
    resolveLandingAfterAbility(player, false);
    return true;
}

bool GameSession::executeLassoCard(Player& player, SkillCard&) {
    const int boardSize = game.getBoardSize();
    std::vector<Player*> candidates;
    for (Player& other : game.getPlayers()) {
        if (&other == &player || other.getStatus() == BANKRUPT) {
            continue;
        }

        const int distance =
            (other.getPosition() - player.getPosition() + boardSize) % boardSize;
        if (distance > 0) {
            candidates.push_back(&other);
        }
    }

    if (candidates.empty()) {
        std::cout << "Tidak ada pemain lawan yang berada di depanmu.\n";
        return false;
    }

    std::cout << "Pilih target LassoCard:\n";
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        const Player* target = candidates[i];
        std::cout << (i + 1) << ". " << target->getUsername() << " ("
                  << board.getTile(target->getPosition()).getCode() << " - "
                  << board.getTile(target->getPosition()).getName() << ")\n";
    }

    const int choice = cli.getInputHandler().readChoice(
        1, static_cast<int>(candidates.size()), "Pilih target LassoCard: ");
    Player* target = candidates[static_cast<std::size_t>(choice - 1)];
    target->setPosition(player.getPosition());
    std::cout << target->getUsername() << " ditarik ke petak "
              << board.getTile(player.getPosition()).getName() << ".\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "LassoCard -> tarik " + target->getUsername());
    resolveLandingAfterAbility(*target, false);
    return true;
}

bool GameSession::executeDemolitionCard(Player& player, SkillCard&) {
    std::vector<Property*> targets;
    for (Player& other : game.getPlayers()) {
        if (&other == &player || other.getStatus() == BANKRUPT) {
            continue;
        }

        for (Property* property : other.getProperties()) {
            if (property != nullptr) {
                targets.push_back(property);
            }
        }
    }

    if (targets.empty()) {
        std::cout << "Tidak ada properti lawan yang bisa dihancurkan.\n";
        return false;
    }

    std::cout << "Pilih properti target DemolitionCard:\n";
    for (std::size_t i = 0; i < targets.size(); ++i) {
        Property* target = targets[i];
        std::cout << (i + 1) << ". " << target->getCode() << " | "
                  << target->getName() << " | pemilik: "
                  << (target->getOwner() ? target->getOwner()->getUsername() : "BANK");
        Street* street = dynamic_cast<Street*>(target);
        if (street != nullptr) {
            std::cout << " | " << buildingLabel(street);
        }
        std::cout << "\n";
    }

    const int choice = cli.getInputHandler().readChoice(
        1, static_cast<int>(targets.size()),
        "Pilih properti untuk DemolitionCard: ");
    Property* target = targets[static_cast<std::size_t>(choice - 1)];
    Player* owner = target->getOwner();
    if (owner != nullptr) {
        owner->removeProperty(target);
    }
    target->setOwner(nullptr);
    target->setStatusStr("BANK");
    target->setFestival(1, 0);
    target->setBuildingCount(0);

    std::cout << target->getName()
              << " dihancurkan dan kembali menjadi milik Bank.\n";
    game.getLogger().log(game.getCurrentTurn(), player.getUsername(), "KARTU",
                         "DemolitionCard -> " + target->getCode());
    return true;
}

void GameSession::resolveLandingAfterAbility(Player& player, bool /*grantGoSalary*/) {
    Tile& tile = board.getTile(player.getPosition());
    tile.onLanded(player, game);
    if (player.getStatus() == JAILED) {
        markPlayerJailed(player);
    }
}

int GameSession::applyDiscountToAmount(const Player& player, int amount) const {
    if (!player.hasDiscount() || amount <= 0) {
        return amount;
    }

    const int effectiveDiscount =
        std::clamp(player.getDiscountPercentage(), 0, 100);
    return amount - (amount * effectiveDiscount / 100);
}

void GameSession::handleSave(const Command& command) {
    if (turnActionTaken) {
        std::cout << "SIMPAN hanya dapat dipanggil di awal giliran.\n";
        return;
    }

    if (saveToFile(command.args[0])) {
        std::cout << "Permainan berhasil disimpan ke: " << command.args[0]
                  << "\n";
        game.getLogger().log(game.getCurrentTurn(),
                             game.getCurrentPlayer().getUsername(), "SAVE",
                             "Simpan ke " + command.args[0]);
        return;
    }

    std::cout << "Gagal menyimpan file! Pastikan direktori dapat ditulis.\n";
}

void GameSession::handleLoad(const Command&) const {
    std::cout << "MUAT hanya dapat dilakukan dari menu awal program.\n";
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
    winners.push_back(&game.getWinner());
    cli.showWinner(winners);
}

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

        const PropertyTile* propertyTile =
            dynamic_cast<const PropertyTile*>(&tile);
        if (propertyTile != nullptr) {
            const Property& property = propertyTile->getProperty();
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

            Street* street = dynamic_cast<Street*>(property);
            if (street != nullptr) {
                oss << " | " << buildingLabel(street);
                if (street->getFMult() > 1) {
                    oss << " | Festival x" << street->getFMult() << " ("
                        << street->getFDur() << "t)";
                }
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

void GameSession::updateStartupState(const std::string& mode,
                                     const std::string& prompt,
                                     int expectedPlayers,
                                     int collectedPlayers) {
    startupMode = mode;
    startupPrompt = prompt;
    startupExpectedPlayers = expectedPlayers;
    startupCollectedPlayers = collectedPlayers;
}

std::vector<Property*> GameSession::getAllProperties() const {
    std::vector<Property*> properties;
    for (int i = 0; i < board.getTileCount(); ++i) {
        const Tile& tile = board.getTile(i);
        const PropertyTile* propertyTile =
            dynamic_cast<const PropertyTile*>(&tile);
        if (propertyTile != nullptr) {
            properties.push_back(
                const_cast<Property*>(&propertyTile->getProperty()));
        }
    }
    return properties;
}

Property* GameSession::findPropertyByCode(const std::string& code) const {
    const std::string normalizedCode = uppercase(code);
    for (Property* property : getAllProperties()) {
        if (property != nullptr &&
            uppercase(property->getCode()) == normalizedCode) {
            return property;
        }
    }
    return nullptr;
}

int GameSession::findTilePositionByCode(const std::string& code) const {
    const std::string normalized = uppercase(code);
    const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
    for (int i = 0; i < board.getTileCount(); ++i) {
        const std::string tileCode =
            i < static_cast<int>(layout.size()) ? layout[static_cast<std::size_t>(i)].code
                                                : board.getTile(i).getCode();
        if (uppercase(tileCode) == normalized) {
            return i;
        }
    }
    return -1;
}

std::vector<Property*> GameSession::getMortgageableProperties() const {
    std::vector<Property*> result;
    const Player& currentPlayer = game.getCurrentPlayer();
    for (Property* property : currentPlayer.getProperties()) {
        if (property == nullptr) {
            continue;
        }
        if (property->getStatus() != PropertyStatus::OWNED) {
            continue;
        }
        if (property->getBuildingCount() > 0) {
            continue;
        }
        result.push_back(property);
    }
    return result;
}

std::vector<Property*> GameSession::getRedeemableProperties() const {
    std::vector<Property*> result;
    const Player& currentPlayer = game.getCurrentPlayer();
    for (Property* property : currentPlayer.getProperties()) {
        if (property != nullptr &&
            property->getStatus() == PropertyStatus::MORTGAGED) {
            result.push_back(property);
        }
    }
    return result;
}

std::vector<Street*> GameSession::getBuildableStreets() const {
    std::vector<Street*> result;
    const Player& currentPlayer = game.getCurrentPlayer();
    for (Property* property : currentPlayer.getProperties()) {
        Street* street = dynamic_cast<Street*>(property);
        if (street == nullptr) {
            continue;
        }
        if (street->getStatus() != PropertyStatus::OWNED) {
            continue;
        }
        if (!street->isMonopolized()) {
            continue;
        }
        result.push_back(street);
    }
    return result;
}

bool GameSession::saveToFile(const std::string& rawFilename) const {
    std::filesystem::path path(rawFilename);
    if (path.extension().empty()) {
        path.replace_extension(".txt");
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    const std::vector<BoardTileConfig>& layout = configuration.getBoardLayout();
    file << game.getCurrentTurn() << " " << game.getMaxTurn() << "\n";
    file << game.getPlayers().size() << "\n";

    for (const Player& player : game.getPlayers()) {
        const int playerPosition = player.getPosition();
        const std::string positionToken =
            playerPosition >= 0 && playerPosition < static_cast<int>(layout.size())
                ? layout[static_cast<std::size_t>(playerPosition)].code
                : std::to_string(playerPosition);
        file << player.getUsername() << " " << player.getCash() << " "
             << positionToken << " " << playerStatusLabel(player) << "\n";
        file << player.getCardCount() << "\n";

        for (Card* card : player.getHand()) {
            const DiscountCard* discountCard =
                dynamic_cast<const DiscountCard*>(card);
            file << card->getType() << " " << card->getValue();
            if (discountCard != nullptr) {
                file << " " << discountCard->getRemainingDuration();
            } else {
                file << " 0";
            }
            file << "\n";
        }
    }

    for (std::size_t i = 0; i < game.getPlayers().size(); ++i) {
        if (i > 0) {
            file << " ";
        }
        file << game.getPlayers()[i].getUsername();
    }
    file << "\n";
    if (!game.getPlayers().empty()) {
        file << game.getCurrentPlayer().getUsername() << "\n";
    } else {
        file << 0 << "\n";
    }

    std::vector<Property*> properties = getAllProperties();
    file << properties.size() << "\n";
    for (Property* property : properties) {
        file << property->getCode() << " " << lowercase(property->getType()) << " ";
        if (property->getOwner() != nullptr) {
            file << property->getOwner()->getUsername();
        } else {
            file << "BANK";
        }
        file << " " << property->getStatusString() << " " << property->getFMult()
             << " " << property->getFDur() << " ";
        if (property->getBuildingCount() >= static_cast<int>(BuildingLevel::HOTEL)) {
            file << "H";
        } else {
            file << property->getBuildingCount();
        }
        file << "\n";
    }

    std::vector<std::string> serializedDeck = skillDeck;
    serializedDeck.insert(serializedDeck.end(), skillDiscard.begin(), skillDiscard.end());
    file << serializedDeck.size() << "\n";
    for (const std::string& cardType : serializedDeck) {
        file << cardType << "\n";
    }

    std::vector<LogEntry> entries = game.getLogger().getEntries();
    file << entries.size() << "\n";
    for (const LogEntry& entry : entries) {
        file << entry.turn << " " << entry.username << " " << entry.actionType
             << " " << entry.detail << "\n";
    }

    return true;
}

bool GameSession::loadFromFile(const std::string& rawFilename) {
    std::filesystem::path path(rawFilename);
    if (path.extension().empty()) {
        path.replace_extension(".txt");
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    int currentTurn = 0;
    int maxTurn = 0;
    if (!(file >> currentTurn >> maxTurn)) {
        return false;
    }

    int playerCount = 0;
    if (!(file >> playerCount) || playerCount < 2 || playerCount > 4) {
        return false;
    }

    std::vector<Player>& players = game.getPlayers();
    players.clear();
    players.reserve(static_cast<std::size_t>(playerCount));
    jailAttemptCounts.clear();

    for (int i = 0; i < playerCount; ++i) {
        std::string username;
        int cash = 0;
        std::string positionToken;
        std::string status;
        if (!(file >> username >> cash >> positionToken >> status)) {
            return false;
        }

        int position = -1;
        const bool numericPosition =
            !positionToken.empty() &&
            std::all_of(positionToken.begin(), positionToken.end(), [](unsigned char ch) {
                return std::isdigit(ch) != 0;
            });
        if (numericPosition) {
            position = std::stoi(positionToken);
        } else {
            position = findTilePositionByCode(positionToken);
        }
        if (position < 0) {
            return false;
        }

        int handCount = 0;
        if (!(file >> handCount)) {
            return false;
        }

        std::vector<Card*> hand;
        for (int c = 0; c < handCount; ++c) {
            std::string type;
            int value = 0;
            int duration = 0;
            if (!(file >> type >> value >> duration)) {
                return false;
            }

            SkillCard* card = createSkillCardInstance(type, value, duration);
            if (card != nullptr) {
                hand.push_back(card);
            }
        }

        PlayerStatus playerStatus = ACTIVE;
        const std::string normalizedStatus = uppercase(status);
        if (normalizedStatus == "BANKRUPT") {
            playerStatus = BANKRUPT;
        } else if (normalizedStatus == "JAILED") {
            playerStatus = JAILED;
        }

        players.emplace_back(username, cash, playerStatus, position, hand);
    }

    std::string turnOrderLine;
    std::getline(file >> std::ws, turnOrderLine);

    std::string activePlayerToken;
    if (!(file >> activePlayerToken)) {
        return false;
    }
    int activePlayerIndex = 0;
    const bool numericActivePlayer =
        !activePlayerToken.empty() &&
        std::all_of(activePlayerToken.begin(), activePlayerToken.end(),
                    [](unsigned char ch) { return std::isdigit(ch) != 0; });
    if (numericActivePlayer) {
        activePlayerIndex = std::stoi(activePlayerToken);
    } else {
        auto activeIt = std::find_if(players.begin(), players.end(),
                                     [&](const Player& player) {
                                         return player.getUsername() == activePlayerToken;
                                     });
        if (activeIt == players.end()) {
            return false;
        }
        activePlayerIndex = static_cast<int>(std::distance(players.begin(), activeIt));
    }

    int propertyCount = 0;
    if (!(file >> propertyCount)) {
        return false;
    }

    for (int i = 0; i < propertyCount; ++i) {
        std::string code;
        std::string type;
        std::string ownerName;
        std::string status;
        int festivalMultiplier = 1;
        int festivalDuration = 0;
        std::string buildingState;

        if (!(file >> code >> type >> ownerName >> status >> festivalMultiplier >>
              festivalDuration >> buildingState)) {
            return false;
        }

        Property* property = findPropertyByCode(code);
        if (property == nullptr) {
            continue;
        }

        property->setStatusStr(status);
        property->setFestival(festivalMultiplier, festivalDuration);
        if (buildingState == "H") {
            property->setBuildingCount(static_cast<int>(BuildingLevel::HOTEL));
        } else {
            property->setBuildingCount(std::stoi(buildingState));
        }

        if (uppercase(ownerName) == "BANK") {
            property->setOwner(nullptr);
            continue;
        }

        auto playerIt = std::find_if(
            players.begin(), players.end(), [&](const Player& player) {
                return player.getUsername() == ownerName;
            });
        if (playerIt != players.end()) {
            property->setOwner(&(*playerIt));
            playerIt->addProperty(property);
        }
    }

    int deckCount = 0;
    if (!(file >> deckCount)) {
        return false;
    }
    skillDeck.clear();
    skillDiscard.clear();
    std::string discardLine;
    std::getline(file, discardLine);
    for (int i = 0; i < deckCount; ++i) {
        std::getline(file, discardLine);
        discardLine = trim(discardLine);
        if (!discardLine.empty()) {
            skillDeck.push_back(discardLine);
        }
    }

    int logCount = 0;
    if (!(file >> logCount)) {
        return false;
    }

    game.getLogger().clear();
    std::getline(file, discardLine);
    for (int i = 0; i < logCount; ++i) {
        std::string line;
        std::getline(file, line);
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.size() < 3) {
            continue;
        }

        const int turn = std::stoi(tokens[0]);
        const std::string username = tokens[1];
        const std::string action = tokens[2];
        std::string detail;
        if (tokens.size() > 3) {
            const std::size_t detailPos = line.find(action);
            if (detailPos != std::string::npos) {
                detail = trim(line.substr(detailPos + action.size()));
            }
        }
        game.getLogger().log(turn, username, action, detail);
    }

    game.setCurrentTurn(currentTurn);
    game.setMaxTurn(maxTurn);
    game.setActivePlayerIndex(activePlayerIndex);
    for (Player& player : players) {
        if (player.getStatus() == JAILED) {
            jailAttemptCounts[player.getUsername()] = 0;
        }
    }
    diceRolledThisTurn = false;
    return true;
}
