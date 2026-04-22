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
                  << " terkena sanksi masuk Penjara.\n";
        game.getLogger().log(game.getCurrentTurn(), currentPlayer.getUsername(),
                             "DOUBLE",
                             "Double ke-3 berturut-turut -> masuk penjara");
        game.goToJail(currentPlayer);
        if (currentPlayer.getStatus() == JAILED) {
            markPlayerJailed(currentPlayer);
        }
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
        GameSessionUtil::uppercase(
            cli.getInputHandler().readToken("Masukkan kode petak: "));

    Property* property = queries.findPropertyByCode(code);
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
    std::vector<Property*> mortgageable = queries.getMortgageableProperties();
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
    if (property->getType() == "Street") {
        Street* street = static_cast<Street*>(property);
        if (game.hasBuildingsInColorGroup(currentPlayer, street->getColorGroup())) {
            std::cout << property->getName()
                      << " tidak dapat digadaikan langsung karena masih ada "
                         "bangunan di color group ["
                      << GameSessionUtil::colorGroupLabel(street->getColorGroup())
                      << "].\n";
            const bool sellBuildings = cli.getInputHandler().readYesNo(
                "Jual semua bangunan color group ini? (y/n): ");
            if (!sellBuildings) {
                return;
            }

            const int soldValue =
                game.sellBuildingsInColorGroup(currentPlayer, street->getColorGroup());
            std::cout << "Bangunan di color group ["
                      << GameSessionUtil::colorGroupLabel(street->getColorGroup())
                      << "] terjual. Kamu menerima M" << soldValue << ".\n";

            const bool continueMortgage = cli.getInputHandler().readYesNo(
                "Lanjut menggadaikan properti ini? (y/n): ");
            if (!continueMortgage) {
                return;
            }
        }
    }

    const int received = game.executeMortgage(currentPlayer, *property);
    if (received <= 0) {
        std::cout << property->getName()
                  << " belum memenuhi syarat untuk digadaikan.\n";
        return;
    }

    std::cout << property->getName() << " berhasil digadaikan.\n";
    std::cout << "Kamu menerima M" << received << " dari Bank.\n";
    std::cout << "Uang kamu sekarang: M" << currentPlayer.getCash() << "\n";
    turnActionTaken = true;
}

void GameSession::handleRedeem() {
    Player& currentPlayer = game.getCurrentPlayer();
    std::vector<Property*> redeemable = queries.getRedeemableProperties();
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
    const int baseRedeemPrice = property->getRedeemPrice();
    const int redeemPrice = game.applyDiscount(currentPlayer, baseRedeemPrice);
    if (!currentPlayer.canPay(redeemPrice)) {
        std::cout << "Uang kamu tidak cukup untuk menebus "
                  << property->getName() << ".\n";
        std::cout << "Harga tebus: M" << redeemPrice << " | Uang kamu: M"
                  << currentPlayer.getCash() << "\n";
        return;
    }

    const int paid = game.executeRedeem(currentPlayer, *property);
    if (paid <= 0) {
        std::cout << "Properti gagal ditebus.\n";
        return;
    }

    std::cout << property->getName() << " berhasil ditebus!\n";
    std::cout << "Kamu membayar M" << baseRedeemPrice;
    if (redeemPrice != baseRedeemPrice) {
        std::cout << " -> M" << redeemPrice << " setelah diskon";
    }
    std::cout << " ke Bank.\n";
    std::cout << "Uang kamu sekarang: M" << currentPlayer.getCash() << "\n";
    turnActionTaken = true;
}

void GameSession::handleBuild() {
    Player& currentPlayer = game.getCurrentPlayer();
    std::map<std::string, std::vector<Property*>> buildableGroups;
    std::map<std::string, std::vector<Street*>> groupedStreets;

    for (Street* street : queries.getBuildableStreets()) {
        if (street == nullptr) {
            continue;
        }

        const std::string label =
            GameSessionUtil::colorGroupLabel(street->getColorGroup());
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
    std::vector<Street*> eligible = game.getEligibleBuildTargets(streets);
    std::cout << "Color group ["
              << orderedGroups[static_cast<std::size_t>(groupChoice - 1)] << "]:\n";
    for (Street* street : streets) {
        const bool canBuild =
            std::find(eligible.begin(), eligible.end(), street) != eligible.end();

        std::cout << "- " << street->getName() << " (" << street->getCode()
                  << ") : " << street->getBuildingLabel();
        if (canBuild) {
            std::cout << " <- dapat dibangun";
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
    const bool hotelUpgrade = target->isNextBuildHotel();
    const int baseCost = target->getNextBuildCost();
    const int cost = game.applyDiscount(currentPlayer, baseCost);

    if (!currentPlayer.canPay(cost)) {
        std::cout << "Uang kamu tidak cukup. Biaya: M" << cost
                  << " | Uang kamu: M" << currentPlayer.getCash() << "\n";
        return;
    }

    const int paid = game.executeBuild(currentPlayer, *target);
    if (paid <= 0) {
        std::cout << "Pembangunan gagal dijalankan.\n";
        return;
    }

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
        "Pilih kartu yang ingin digunakan (0 untuk batal): ");
    if (choice == 0) {
        return;
    }

    SkillCard* selected =
        static_cast<SkillCard*>(hand[static_cast<std::size_t>(choice - 1)]);
    if (selected == nullptr) {
        std::cout << "Kartu terpilih tidak valid.\n";
        return;
    }

    if (!executeSkillCard(currentPlayer, selected)) {
        return;
    }

    turnActionTaken = true;

    if (currentPlayer.getStatus() == JAILED || currentPlayer.getStatus() == BANKRUPT) {
        finishTurn();
    }
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

